#!/usr/bin/env python3
# coding: utf-8

import argparse
import os.path

# The source for the Wren modules that are built into the VM or CLI are turned
# include C string literals. This way they can be compiled directly into the
# code so that file IO is not needed to find and read them.
#
# These string literals are stored in files with a ".wren.inc" extension and
# #included directly by other source files. This generates a ".wren.inc" file
# given a ".wren" module.

PREAMBLE = """// Generated automatically from {0}. Do not edit.
static const char* {1}ModuleSource =
{2};
"""

def wren_to_c_string(input_path, wren_source_lines, module):
  wren_source = ""
  for line in wren_source_lines:
    line = line.replace('"', "\\\"")
    line = line.replace("\n", "\\n\"")
    if wren_source: wren_source += "\n"
    wren_source += '"' + line

  return PREAMBLE.format(input_path, module, wren_source)


def main():
  parser = argparse.ArgumentParser(
      description="Convert a Wren library to a C string literal.")
  parser.add_argument("output", help="The output file to write")
  parser.add_argument("input", help="The source .wren file")

  args = parser.parse_args()

  with open(args.input, "r") as f:
    wren_source_lines = f.readlines()

  module = os.path.splitext(os.path.basename(args.input))[0]
  module = module.replace("opt_", "")
  module = module.replace("wren_", "")

  c_source = wren_to_c_string(args.input, wren_source_lines, module)

  with open(args.output, "w") as f:
    f.write(c_source)


main()
