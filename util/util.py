# Utility functions used by other Python files in this directory.

import os
import os.path
import platform
import shutil
import subprocess
import sys

def python2_binary():
  """Tries to find a python 2 executable."""

  # Using [0] instead of .major here to support Python 2.6.
  if sys.version_info[0] == 2:
    return sys.executable or "python"
  else:
    return "python2"


def clean_dir(dir):
  """If dir exists, deletes it and recreates it, otherwise creates it."""
  if os.path.isdir(dir):
    remove_dir(dir)

  os.makedirs(dir)


def ensure_dir(dir):
  """Creates dir if not already there."""

  if os.path.isdir(dir):
    return

  os.makedirs(dir)


def remove_dir(dir):
  """Recursively removes dir."""

  if platform.system() == "Windows":
    # rmtree gives up on readonly files on Windows
    # rd doesn't like paths with forward slashes
    subprocess.check_call(
        ['cmd', '/c', 'rd', '/s', '/q', dir.replace('/', '\\')])
  else:
    shutil.rmtree(dir)


def replace_in_file(path, text, replace):
  """Replaces all occurrences of `text` in the file at `path` with `replace`."""
  with open(path) as file:
    contents = file.read()

  contents = contents.replace(text, replace)

  with open(path, "w") as file:
    file.write(contents)


def run(args, cwd=None):
  """Spawn a process to invoke [args] and mute its output."""

  try:
    # check_output() was added in Python 2.7.
    has_check_output = (sys.version_info[0] > 2 or
        (sys.version_info[0] == 2 and sys.version_info[1] >= 7))

    if has_check_output:
      subprocess.check_output(args, cwd=cwd, stderr=subprocess.STDOUT)
    else:
      proc = subprocess.Popen(args, cwd=cwd, stdout=subprocess.PIPE)
      proc.communicate()[0].split()
  except subprocess.CalledProcessError as error:
    print(error.output)
    sys.exit(error.returncode)
