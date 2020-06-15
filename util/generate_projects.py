#!/usr/bin/env python

import sys
from os import getenv, path
from subprocess import PIPE, run
import platform

PREMAKE_DIR = path.join(path.dirname(__file__), "../projects/premake")

# Default binary name
PREMAKE_BIN = "premake5"
if platform.system() == "Windows":
  PREMAKE_BIN += ".exe"

# We try the env first, as that's absolute.
# If not found we try the 'intended' approach,
# of placing a premake binary alongside premake5.lua.
# If that isn't found, attempt the plain binary name.
premake = getenv("WREN_PREMAKE", None)
if premake is None:
  premake = PREMAKE_BIN
  premake_local = path.join(PREMAKE_DIR, PREMAKE_BIN)
  if path.isfile(premake_local):
    print("Using local premake in 'projects/premake' ...")
    premake = premake_local
else:
  print("Using premake from 'WREN_PREMAKE' env ...")

def run_premake(action, os):
  run([premake, action, "--os=" + os], cwd=PREMAKE_DIR)

try:

  run_premake("gmake2", "bsd")
  run_premake("gmake2", "linux")
  run_premake("vs2017", "windows")
  run_premake("vs2019", "windows")
  run_premake("gmake2", "macosx")
  run_premake("xcode4", "macosx")

except Exception as e:

  print("Unable to run premake, while trying the binary '" + premake + "' ...")
  print("  reason: " + str(e))
  print("\nIf premake can't be found, possible options are:")
  print("- Set the env variable 'WREN_PREMAKE' to the path to a premake binary")
  print("- Place a premake5 binary for your host platform in projects/premake")
  print("- Add a location with a premake5 binary to the PATH")

  exit(1)