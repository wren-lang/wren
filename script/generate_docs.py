#!/usr/bin/env python

import glob
import markdown
import os
import shutil
import subprocess
import sys
import time
from datetime import datetime

def is_up_to_date(path, out_path):
  # See if it's up to date.
  source_mod = os.path.getmtime(path)
  source_mod = max(source_mod, os.path.getmtime('doc/site/template.html'))

  dest_mod = 0
  if os.path.exists(out_path):
    dest_mod = os.path.getmtime(out_path)
  return source_mod < dest_mod

def format_file(path, skip_up_to_date):
  basename = os.path.basename(path)
  basename = basename.split('.')[0]

  out_path = "build/docs/" + basename + ".html"

  if skip_up_to_date and is_up_to_date(path, out_path):
    # It's up to date.
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

      elif stripped.startswith('#'):
        # Add anchors to the headers.
        index = stripped.find(" ")
        headertype = stripped[:index]
        header = stripped[index:].strip()
        anchor = header.lower().replace(' ', '-')
        anchor = anchor.translate(None, '.?!:/')

        contents += indentation + headertype
        contents += '{1} <a href="#{0}" name="{0}" class="header-anchor">#</a>\n'.format(anchor, header)

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


def check_sass():
    source_mod = os.path.getmtime('doc/site/style.scss')

    dest_mod = 0
    if os.path.exists('build/docs/style.css'):
      dest_mod = os.path.getmtime('build/docs/style.css')

    if source_mod < dest_mod:
        return

    subprocess.call(['sass', 'doc/site/style.scss', 'build/docs/style.css'])
    print "built css"


def format_files(skip_up_to_date):
  check_sass()

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

# Watch files.
if len(sys.argv) == 2 and sys.argv[1] == '--watch':
  while True:
    format_files(True)
    time.sleep(0.3)
