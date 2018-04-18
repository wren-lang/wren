#!/usr/bin/env python3

import codecs
import glob
import fnmatch
import os
import re

TODO_PATTERN = re.compile(r'\s*// TODO:')
DOC_PATTERN = re.compile(r'\s*//')
EXPECT_PATTERN = re.compile(r'// expect')

C_FORMAT_LINE = "{0:<10}  {1:>7}  {2:>7}  {3:>7}  {4:>7}  {5:>7}  {6:>7}  {7:>7}"
WREN_FORMAT_LINE = "{0:<10}  {1:>7}  {2:>7}  {3:>7}  {4:>7}  {5:>7}  {6:>7}"

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
num_benchmark_files = 0
num_benchmark_todos = 0
num_benchmark_empty = 0
num_benchmark = 0

def c_metrics(label, directories):
  """Reports the metrics of one or more directories of C code."""
  num_files = 0
  num_semicolons = 0
  num_todos = 0
  num_code = 0
  num_docs = 0
  num_empty = 0

  for directory in directories:
    files = glob.iglob(directory + "/*.[ch]")
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

          stripped = line.strip()
          # Don't count { or } lines since Wren's coding style puts them on
          # their own lines but they don't add anything meaningful to the
          # length of the program.
          if stripped == "" or stripped == "{" or stripped == "}":
            num_empty += 1
            continue

          num_code += 1

  print(C_FORMAT_LINE.format(
      label, num_files, num_semicolons, num_todos, num_code, num_docs,
      num_empty, num_todos + num_docs + num_empty + num_code))


def wren_metrics(label, directories):
  """Reports the metrics of one or more directories of Wren code."""
  num_files = 0
  num_empty = 0
  num_code = 0
  num_todos = 0
  num_expects = 0

  for directory in directories:
    for dir_path, dir_names, file_names in os.walk(directory):
      for file_name in fnmatch.filter(file_names, "*.wren"):
        num_files += 1

        # NOTE: In the old Python 2 version of the script, files were opened as ISO-8859-1
        with codecs.open(os.path.join(dir_path, file_name), "r", encoding="utf-8", errors="ignore") as input:
          for line in input:
            if line.strip() == "":
              num_empty += 1
              continue

            match = TODO_PATTERN.match(line)
            if match:
              num_todos += 1
              continue

            match = EXPECT_PATTERN.search(line)
            if match:
              num_expects += 1
              continue

            num_code += 1

  print(WREN_FORMAT_LINE.format(
      label, num_files, num_todos, num_code, num_expects, num_empty,
      num_todos + num_code + num_expects + num_empty))


print(C_FORMAT_LINE.format(
    "", "files", "';'", "todos", "code", "comment", "empty", "total"))
c_metrics("vm",       ["src/vm", "src/include"])
c_metrics("optional", ["src/optional"])
c_metrics("cli",      ["src/cli", "src/module"])

print()
print(WREN_FORMAT_LINE.format(
    "", "files", "todos", "code", "expects", "empty", "total"))
wren_metrics("core",      ["src/vm"])
wren_metrics("optional",  ["src/optional"])
wren_metrics("cli",       ["src/module"])
wren_metrics("test",      ["test"])
