#!/usr/bin/python
import re

with open("corelib.wren", "r") as f:
  lines = f.readlines()

# Remove the comment from the top.
lines.pop(0)
lines.pop(0)

corelib = ""
for line in lines:
  line = line.replace('"', "\\\"")
  line = line.replace("\n", "\\n\"")
  if corelib: corelib += "\n"
  corelib += '"' + line

with open("src/wren_core.c", "r") as f:
  wren_core = f.read()

# re.sub() will unescape escape sequences, but we want them to stay as escapes
# in the C string literal.
corelib = corelib.replace('\\', '\\\\')

corelib = "coreLibSource =\n" + corelib + ";"

PATTERN = re.compile(r'coreLibSource =\n("(.|[\n])*?);')
wren_core = PATTERN.sub(corelib, wren_core)

with open("src/wren_core.c", "w") as f:
  f.write(wren_core)
