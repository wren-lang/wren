#!/usr/bin/python2

import glob
import fnmatch
import itertools
import os
import re

TODO_PATTERN = re.compile(r'\s*// TODO:')
DOC_PATTERN = re.compile(r'\s*//')
EXPECT_PATTERN = re.compile(r'// expect')

num_files = 0
num_docs = 0
num_code = 0
num_empty = 0
num_todos = 0
num_semicolons = 0
num_test_files = 0
num_test_todos = 0
num_expects = 0
num_test_empty = 0
num_test = 0

files = itertools.chain(glob.iglob("src/*.[ch]"), glob.iglob("include/*.[ch]"))
for source_path in files:
  num_files += 1
  with open(source_path, "r") as input:
    for line in input:
      num_semicolons += line.count(';')
      match = TODO_PATTERN.match(line)
      if match:
        num_todos += 1
        continue

      match = DOC_PATTERN.match(line)
      if match:
        num_docs += 1
        continue

      if (line.strip() == ""):
        num_empty += 1
        continue

      num_code += 1

for dir_path, dir_names, file_names in os.walk("test"):
  for file_name in fnmatch.filter(file_names, "*.wren"):
    num_test_files += 1
    with open(os.path.join(dir_path, file_name), "r") as input:
      for line in input:
        if (line.strip() == ""):
          num_test_empty += 1
        else:
          num_test += 1

        match = TODO_PATTERN.match(line)
        if match:
          num_test_todos += 1
          continue

        match = EXPECT_PATTERN.search(line)
        if match:
          num_expects += 1
          continue

print "source:"
print "  files          ", num_files
print "  semicolons     ", num_semicolons
print "  TODOs          ", num_todos
print "  comment lines  ", num_docs
print "  code lines     ", num_code
print "  empty lines    ", num_empty
print
print "test:"
print "  files          ", num_test_files
print "  TODOs          ", num_test_todos
print "  expectations   ", num_expects
print "  non-empty lines", num_test
print "  empty lines    ", num_test_empty
