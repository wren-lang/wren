#!/usr/bin/env python

import sys
import os.path
import re

INCLUDE_PATTERN = re.compile(r'^\s*#include "([\w.]+)"')

seen_files = set()

def add_file(filename):
  basename = os.path.basename(filename)
  # Only include each file at most once.
  if basename in seen_files:
    return
  seen_files.add(basename)
  path = os.path.dirname(filename)

  with open(filename, 'r') as f:
    for line in f:
      m = INCLUDE_PATTERN.match(line)
      if m:
        add_file(os.path.join(path, m.group(1)))
      else:
        sys.stdout.write(line)

for f in sys.argv[1:]:
  add_file(f)
