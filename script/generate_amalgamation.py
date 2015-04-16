#!/usr/bin/env python

import sys
from os.path import basename, dirname, join
import re

INCLUDE_PATTERN = re.compile(r'^\s*#include "([\w.]+)"')

seen_files = set()
out = sys.stdout

def add_file(filename):
  bname = basename(filename)
  # Only include each file at most once.
  if bname in seen_files:
    return
  seen_files.add(bname)
  path = dirname(filename)

  out.write('// Begin file "{0}"\n'.format(filename))
  with open(filename, 'r') as f:
    for line in f:
      m = INCLUDE_PATTERN.match(line)
      if m:
        add_file(join(path, m.group(1)))
      else:
        out.write(line)
  out.write('// End file "{0}"\n'.format(filename))

for f in sys.argv[1:]:
  add_file(f)
