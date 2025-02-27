#!/usr/bin/env python

from __future__ import print_function

from argparse import ArgumentParser
from collections import defaultdict
from os import listdir
from os.path import abspath, dirname, isdir, isfile, join, realpath, relpath, splitext
import re
from subprocess import Popen, PIPE
import sys
from threading import Timer
import platform
# Runs the tests.

parser = ArgumentParser()
parser.add_argument('--suffix', default='',
  help='suffix to the executable name; e.g. "_d"')
parser.add_argument('--myself', action='store_true',
  help='for verifying the test runner itself')
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('suite', nargs='?',
  help='a string prefix that filters the path to files,'
    + ' relative from test/ directory; e.g. "core/map/key_" or "../example"')

args = parser.parse_args(sys.argv[1:])

WREN_DIR = dirname(dirname(realpath(__file__)))
WREN_APP = join(WREN_DIR, 'bin', 'wren_test' + args.suffix)

WREN_APP_WITH_EXT = WREN_APP
if platform.system() == "Windows":
  WREN_APP_WITH_EXT += ".exe"

if not isfile(WREN_APP_WITH_EXT):
  print("The binary file was not found; expected it to be: " + WREN_APP)
  print("In order to run the tests, you need to build Wren first!")
  sys.exit(1)

# print("Wren Test Directory - " + WREN_DIR)
# print("Wren Test App - " + WREN_APP)

EXPECT_PATTERN               = re.compile(r'// expect: ?(.*)')
EXPECT_ERROR_PATTERN         = re.compile(r'// expect error(?! line)')
EXPECT_ERROR_LINE_PATTERN    = re.compile(r'// expect error line (\d+)')
EXPECT_RUNTIME_ERROR_PATTERN = re.compile(r'// expect (handled )?runtime error: (.+)')

STDIN_PATTERN   = re.compile(r'// stdin: (.*)')
SKIP_PATTERN    = re.compile(r'// skip: (.*)')
NONTEST_PATTERN = re.compile(r'// nontest')

# Patterns for output
EOL_PATTERN         = re.compile(r'\n|\r\n')
ERROR_PATTERN       = re.compile(r'\[.* line (\d+)\] Error')
STACK_TRACE_PATTERN = re.compile(r'(?:\[\./)?test/.* line (\d+)\] in')

passed = 0
failed = 0
num_skipped = 0
skipped = defaultdict(int)
expectations = 0

def split_into_lines(string):
  lines = string.split('\n')
  # Remove the trailing last empty line.
  if lines[-1] == '':
    del lines[-1]
  return lines

class Test:
  def __init__(self, path):
    self.path = path
    self.output = []
    self.compile_errors_line_num = set()
    self.runtime_error_line_num = 0
    self.runtime_error_message = None
    self.exit_code = 0
    self.input_bytes = None
    self.failures = []


  def parse(self):
    global num_skipped
    global skipped
    global expectations

    input_lines = []
    line_num = 1

    # Note #1: we have unicode tests that require utf-8 decoding.
    # Note #2: python `open` on 3.x modifies contents regarding newlines.
    # To prevent this, we specify newline='' and we don't use the
    # readlines/splitlines/etc family of functions, these
    # employ the universal newlines concept which does this.
    # We have tests that embed \r and \r\n for validation, all of which
    # get manipulated in a not helpful way by these APIs.

    with open(self.path, 'r', encoding="utf-8", newline='', errors='replace') as file:
      data = file.read()
      lines = EOL_PATTERN.split(data)
      for line in lines:
        if len(line) <= 0:
          line_num += 1
          continue

        match = EXPECT_PATTERN.search(line)
        if match:
          self.output.append((match.group(1), line_num))
          expectations += 1

        match = EXPECT_ERROR_PATTERN.search(line)
        if match:
          self.compile_errors_line_num.add(line_num)

          # If we expect a compile error, it should exit with WREN_EX_DATAERR.
          self.exit_code = 65
          expectations += 1

        match = EXPECT_ERROR_LINE_PATTERN.search(line)
        if match:
          self.compile_errors_line_num.add(int(match.group(1)))

          # If we expect a compile error, it should exit with WREN_EX_DATAERR.
          self.exit_code = 65
          expectations += 1

        match = EXPECT_RUNTIME_ERROR_PATTERN.search(line)
        if match:
          self.runtime_error_line_num = line_num
          self.runtime_error_message = match.group(2)
          # If the runtime error isn't handled, it should exit with WREN_EX_SOFTWARE.
          if match.group(1) != "handled ":
            self.exit_code = 70
          expectations += 1

        match = STDIN_PATTERN.search(line)
        if match:
          input_lines.append(match.group(1))

        match = SKIP_PATTERN.search(line)
        if match:
          num_skipped += 1
          skipped[match.group(1)] += 1
          return False

        # Not a test file at all, so ignore it.
        match = NONTEST_PATTERN.search(line)
        if match:
          return False

        line_num += 1


    # If any input is fed to the test in stdin, concatenate it into one string.
    if input_lines:
      self.input_bytes = "\n".join(input_lines).encode("utf-8")

    # If we got here, it's a valid test.
    return True


  def run(self, app, type, env):
    # Invoke wren and run the test.
    test_arg = self.path
    proc = Popen([app, test_arg], stdin=PIPE, stdout=PIPE, stderr=PIPE, env=env)

    # If a test is too long, kill it.
    #
    # This is mainly useful for running the tests while stress testing the GC,
    # which can make a few pathological tests much slower.
    timed_out = [False]
    def kill_process(p):
      timed_out[0] = True
      p.kill()

    timer = Timer(5, kill_process, [proc])

    try:
      timer.start()
      out, err = proc.communicate(self.input_bytes)

      if timed_out[0]:
        self.fail("Timed out.")
      else:
        self.validate(type == "example", proc.returncode, out, err)
    finally:
      timer.cancel()


  def validate(self, is_example, exit_code, out, err):
    if self.compile_errors_line_num and self.runtime_error_message:
      self.fail("BAD TEST: Cannot expect both compile and runtime errors.")
      return

    try:
      out = out.decode("utf-8").replace('\r\n', '\n')
      err = err.decode("utf-8").replace('\r\n', '\n')
    except UnicodeDecodeError:
      self.fail('Error decoding output.')
      return

    error_lines = split_into_lines(err)

    if self.runtime_error_message:
      self.validate_runtime_error(error_lines)
    else:
      self.validate_compile_errors(error_lines)

    self.validate_exit_code(exit_code, error_lines)

    # Ignore output from examples.
    if is_example: return

    self.validate_output(out)


  def validate_runtime_error(self, error_lines):
    # Skip any compile errors. This can happen if there is a compile error in
    # a module loaded by the module being tested.
    line_num = 0
    while line_num < len(error_lines) and ERROR_PATTERN.search(error_lines[line_num]):
      line_num += 1

    if not line_num < len(error_lines):
      self.fail('Expected runtime error "{0}" and got none.',
          self.runtime_error_message)
      return

    if error_lines[line_num] != self.runtime_error_message:
      self.fail('Expected runtime error "{0}" and got:',
          self.runtime_error_message)
      self.fail('    ' + error_lines[line_num])

    # Make sure the stack trace has the right line. Skip over any lines that
    # come from builtin libraries.
    match = False
    stack_lines = error_lines[line_num + 1:]
    for stack_line in stack_lines:
      match = STACK_TRACE_PATTERN.search(stack_line)
      if match: break

    if not match:
      self.fail('Expected stack trace and got:')
      for stack_line in stack_lines:
        self.fail('    ' + stack_line)
    else:
      line_num = int(match.group(1))
      if line_num != self.runtime_error_line_num:
        self.fail('Expected runtime error on line {0} but was on line {1}.',
            self.runtime_error_line_num, line_num)


  def validate_compile_errors(self, error_lines):
    # Validate that every compile error was expected.
    found = set()
    first = True
    for line in error_lines:
      match = ERROR_PATTERN.search(line)
      if match:
        error_line_num = float(match.group(1))
        if error_line_num in self.compile_errors_line_num:
          found.add(error_line_num)
        else:
          self.fail('Unexpected error:')
          self.fail('    ' + line)
      elif line != '':
        if first:
          self.fail('Unexpected output on stderr:')
          first = False
        self.fail('    ' + line)

    # Validate that every expected error occurred.
    for line_num in self.compile_errors_line_num - found:
      self.fail('Missing expected error on line {0}.', line_num)


  def validate_exit_code(self, exit_code, error_lines):
    if exit_code == self.exit_code: return

    self.fail('Expected return code {0} and got {1}. Stderr:',
        self.exit_code, exit_code)
    self.failures += [ '  /-----' ]
    self.failures += [ '  | ' + l for l in error_lines ]
    self.failures += [ '  \-----' ]


  def validate_output(self, out):
    out_lines = split_into_lines(out)
    index = 0
    for line in out_lines:
      if sys.version_info < (3, 0):
        line = line.encode('utf-8')

      if index >= len(self.output):
        self.fail('Got output "{0}" when none was expected.', line)
      elif self.output[index][0] != line:
        self.fail('Expected output "{0}" on line {1} and got "{2}".',
            self.output[index][0], self.output[index][1], line)
      index += 1

    while index < len(self.output):
      self.fail('Missing expected output "{0}" on line {1}.',
          self.output[index][0], self.output[index][1])
      index += 1


  def fail(self, message, *args):
    if args:
      message = message.format(*args)
    self.failures.append(message)


def color_text(text, color):
  """Converts text to a string and wraps it in the ANSI escape sequence for
  color, if supported."""

  # No ANSI escapes on Windows.
  if sys.platform == 'win32':
    return str(text)

  return color + str(text) + '\033[0m'


def green(text):  return color_text(text, '\033[32m')
def pink(text):   return color_text(text, '\033[91m')
def red(text):    return color_text(text, '\033[31m')
def yellow(text): return color_text(text, '\033[33m')


def walk(dir, callback, ignored=None):
  """
  Walks [dir], and executes [callback] on each file unless it is an [ignored] one.
  """

  if ignored is None:
    ignored = []
  ignored += [".",".."]

  dir = abspath(dir)
  for file in [file for file in listdir(dir) if not file in ignored]:
    nfile = join(dir, file)
    if isdir(nfile):
      walk(nfile, callback)
    else:
      callback(nfile)


def print_line(line=None):
  # Erase the line.
  print('\033[2K', end='')
  # Move the cursor to the beginning.
  print('\r', end='')
  if line:
    print(line, end='')
    sys.stdout.flush()


def run_script(app, path, type):
  global passed
  global failed
  global num_skipped

  if splitext(path)[1] != '.wren':
    return

  # Check if we are just running a subset of the tests.
  if args.suite:
    this_test = relpath(path, join(WREN_DIR, 'test'))
    if not this_test.startswith(args.suite):
      return

  # Update the status line.
  print_line('({}) Passed: {} Failed: {} Skipped: {} '.format(
      relpath(app, WREN_DIR), green(passed), red(failed), yellow(num_skipped)))

  # Make a nice short path relative to the working directory.

  # Normalize it to use "/" since, among other things, wren expects its argument
  # to use that.
  path = relpath(path).replace("\\", "/")

  # Read the test and parse out the expectations.
  test = Test(path)

  torun = test.parse()

  if args.verbose:
    messages = [path]
    if not torun:
      messages += [yellow('(SKIP)')]
    print(*messages)

  if not torun:
    # It's a skipped test or non-test file.
    return

  env = None
  if False:
    f = "bytecode_test.wrenb"
    # f = 'bytecode/' + path.replace('/', '-') + 'b'
    f_to = f
    f_from = f

    env = {
      "WREN_SNAPSHOT":      "yfsr",
      "WREN_SNAPSHOT_TO":   f_to,
      "WREN_SNAPSHOT_FROM": f_from,
    }

    # Truncate the target file; hence an empty file matches a non-compilable
    # source file.
    with open(f_to, 'w') as file: pass

  test.run(app, type, env)

  # Display the failures.
  if len(test.failures) == 0:
    passed += 1
  else:
    failed += 1
    print_line(red('FAIL') + ': ' + path)
    print('')
    for failure in test.failures:
      print('      ' + pink(failure))
    print('')



def run_test(path):
  run_script(WREN_APP, path, "test")


def run_api_test(path):
  run_script(WREN_APP, path, "api test")


def run_example(path):
  # This one is annoyingly slow.
  if "skynet" in path: return

  run_script(WREN_APP, path, "example")


if args.myself:
  # Exercises all kinds of failure, to see how well the runner handle them.
  walk(join(WREN_DIR, 'runner_test'), run_test)
else:
  walk(join(WREN_DIR, 'test'), run_test, ignored=['api', 'benchmark'])
  walk(join(WREN_DIR, 'test', 'api'), run_api_test)
  walk(join(WREN_DIR, 'example'), run_example)

print_line()
if failed == 0:
  print('All ' + green(passed) + ' tests passed (' + str(expectations) +
        ' expectations).')
else:
  print(green(passed) + ' tests passed. ' + red(failed) + ' tests failed.')

for key in sorted(skipped.keys()):
  print('Skipped ' + yellow(skipped[key]) + ' tests: ' + key)

if failed != 0:
  sys.exit(1)
