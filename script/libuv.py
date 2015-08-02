#!/usr/bin/env python

# Downloads and compiles libuv.

from __future__ import print_function

import os
import os.path
import platform
import shutil
import subprocess
import sys

LIB_UV_VERSION = "v1.6.1"
LIB_UV_DIR = "build/libuv"

def ensure_dir(dir):
  """Creates dir if not already there."""

  if os.path.isdir(dir):
    return

  os.makedirs(dir)


def download_libuv():
  """Clones libuv into build/libuv and checks out the right version."""

  # Delete it if already there so we ensure we get the correct version if the
  # version number in this script changes.
  if os.path.isdir(LIB_UV_DIR):
    print("Cleaning output directory...")
    shutil.rmtree(LIB_UV_DIR)

  ensure_dir("build")

  print("Cloning libuv...")
  run([
    "git", "clone", "--quiet", "--depth=1",
    "https://github.com/libuv/libuv.git",
    LIB_UV_DIR
  ])

  print("Getting tags...")
  run([
    "git", "fetch", "--quiet", "--depth=1", "--tags"
  ], cwd=LIB_UV_DIR)

  print("Checking out libuv " + LIB_UV_VERSION + "...")
  run([
    "git", "checkout", "--quiet", LIB_UV_VERSION
  ], cwd=LIB_UV_DIR)

  # TODO: Pin gyp to a known-good commit. Update a previously downloaded gyp
  # if it doesn't match that commit.
  print("Downloading gyp...")
  run([
    "git", "clone", "--quiet", "--depth=1",
    "https://chromium.googlesource.com/external/gyp.git",
    LIB_UV_DIR + "/build/gyp"
  ])


def build_libuv_mac():
  # Create the XCode project.
  run([
    "python", LIB_UV_DIR + "/gyp_uv.py", "-f", "xcode"
  ])

  # Compile it.
  # TODO: Support debug builds too.
  run([
    "xcodebuild",
    "-ARCHS=\"x86_64\"",
    "-project", LIB_UV_DIR + "/uv.xcodeproj",
    "-configuration", "Release",
    "-target", "All"
  ])


def build_libuv_linux():
  run(["python", LIB_UV_DIR + "/gyp_uv.py", "-f", "make"])
  run(["make", "-C", "out"])


def build_libuv_windows():
  # TODO: Implement me!
  print("Building for Windows not implemented yet.")
  sys.exit(1)


def build_libuv():
  if platform.system() == "Darwin":
    build_libuv_mac()
  elif platform.system() == "Linux":
    build_libuv_linux()
  elif platform.system() == "Windows":
    build_libuv_windows()
  else:
    print("Unsupported platform: " + platform.system())
    sys.exit(1)


def run(args, cwd=None):
  """Spawn a process to invoke [args] and mute its output."""
  subprocess.check_output(args, cwd=cwd, stderr=subprocess.STDOUT)


def main():
  download_libuv()
  build_libuv()


main()
