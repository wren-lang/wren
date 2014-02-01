#!/usr/bin/env python

import glob
import markdown
import os
import shutil
import sys
import time
from datetime import datetime

def format_file(path, skip_up_to_date):
  basename = os.path.basename(path)
  basename = basename.split('.')[0]

  out_path = "build/docs/" + basename + ".html"

  # See if it's up to date.
  source_mod = os.path.getmtime(path)
  source_mod = max(source_mod, os.path.getmtime('doc/site/template.html'))

  dest_mod = 0
  if os.path.exists(out_path):
    dest_mod = os.path.getmtime(out_path)

  if skip_up_to_date and source_mod < dest_mod:
    return

  title = ""

  # Read the markdown file and preprocess it.
  contents = ""
  with open(path, "r") as input:
    # Read each line, preprocessing the special codes.
    for line in input:
      stripped = line.lstrip()
      indentation = line[:len(line) - len(stripped)]

      if stripped.startswith("^"):
        command,_,args = stripped.rstrip("\n").lstrip("^").partition(" ")
        args = args.strip()

        if command == "title":
          title = args
        else:
          print "UNKNOWN COMMAND:", command, args

      else:
        contents = contents + line

  html = markdown.markdown(contents, ['def_list', 'codehilite'])

  modified = datetime.fromtimestamp(os.path.getmtime(path))
  mod_str = modified.strftime('%B %d, %Y')

  fields = {'title': title, 'html': html, 'mod': mod_str}

  with open("doc/site/template.html") as f:
    template = f.read()

  # Write the html output.
  with open(out_path, 'w') as out:
    out.write(template.format(**fields))

  print "converted", basename


def format_files(skip_up_to_date):
  for f in glob.iglob("doc/site/*.markdown"):
    format_file(f, skip_up_to_date)


# Clean the output directory.
if not os.path.exists("build"):
  os.mkdir("build")

if os.path.exists("build/docs"):
  shutil.rmtree("build/docs")
os.mkdir("build/docs")

# Process each markdown file.
format_files(False)

# Copy the CSS file.
shutil.copyfile("doc/site/style.css", "build/docs/style.css")

# TODO(bob): Check for CSS modification.
if len(sys.argv) == 2 and sys.argv[1] == '--watch':
  while True:
    format_files(True)
    time.sleep(0.3)
