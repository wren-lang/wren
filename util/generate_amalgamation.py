#!/usr/bin/env python3

import sys
from os.path import basename, dirname, join, realpath
from glob import iglob
import re

INCLUDE_PATTERN = re.compile(r'^\s*#include "([\w.]+)"')
GUARD_PATTERN = re.compile(r'^#ifndef wren(_\w+)?_h$')
WREN_DIR = dirname(dirname(realpath(__file__)))

seen_files = set()
out = sys.stdout

# Prints a plain text file, adding comment markers.
def add_comment_file(filename):
  with open(filename, 'r') as f:
    for line in f:
      out.write('// ')
      out.write(line)

# Prints the given C source file, recursively resolving local #includes.
def add_file(filename):
  bname = basename(filename)
  # Only include each file at most once.
  if bname in seen_files:
    return
  once = False
  path = dirname(filename)

  out.write('// Begin file "{0}"\n'.format(filename))
  with open(filename, 'r') as f:
    for line in f:
      m = INCLUDE_PATTERN.match(line)
      if m:
        add_file(join(path, m.group(1)))
      else:
        out.write(line)
      if GUARD_PATTERN.match(line):
        once = True
  out.write('// End file "{0}"\n'.format(filename))

  # Only skip header files which use #ifndef guards.
  # This is necessary because of the X Macro technique.
  if once:
    seen_files.add(bname)

# Print license on top.
add_comment_file(join(WREN_DIR, 'LICENSE'))
out.write('\n')

# Source files.
add_file(join(WREN_DIR, 'src', 'include', 'wren.h'))

# Must be included here because of conditional compilation.
add_file(join(WREN_DIR, 'src', 'vm', 'wren_debug.h'))

for f in iglob(join(WREN_DIR, 'src', 'vm', '*.c')):
  add_file(f)
