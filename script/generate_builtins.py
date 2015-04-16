#!/usr/bin/env python

import glob
import os.path
import re

PATTERN = re.compile(r'LibSource =\n("(.|[\n])*?);')

def copy_builtin(filename):
  name = os.path.basename(filename)
  name = name.split('.')[0]

  with open(filename, "r") as f:
    lines = f.readlines()

  wren_source = ""
  for line in lines:
    line = line.replace('"', "\\\"")
    line = line.replace("\n", "\\n\"")
    if wren_source: wren_source += "\n"
    wren_source += '"' + line

  # re.sub() will unescape escape sequences, but we want them to stay escapes
  # in the C string literal.
  wren_source = wren_source.replace('\\', '\\\\')

  constant = "LibSource =\n" + wren_source + ";"

  with open("src/vm/wren_" + name + ".c", "r") as f:
    c_source = f.read()

  c_source = PATTERN.sub(constant, c_source)

  with open("src/vm/wren_" + name + ".c", "w") as f:
    f.write(c_source)

  print(name)


for f in glob.iglob("builtin/*.wren"):
  copy_builtin(f)
