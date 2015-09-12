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
LIB_UV_DIR = "deps/libuv"

def ensure_dir(dir):
  """Creates dir if not already there."""

  if os.path.isdir(dir):
    return

  os.makedirs(dir)


def remove_dir(dir):
  """Recursively removes dir."""

  if platform.system() == "Windows":
    # rmtree gives up on readonly files on Windows
    # rd doesn't like paths with forward slashes
    subprocess.check_call(
        ['cmd', '/c', 'rd', '/s', '/q', dir.replace('/', '\\')])
  else:
    shutil.rmtree(dir)


def download_libuv():
  """Clones libuv into deps/libuv and checks out the right version."""

  # Delete it if already there so we ensure we get the correct version if the
  # version number in this script changes.
  if os.path.isdir(LIB_UV_DIR):
    print("Cleaning output directory...")
    remove_dir(LIB_UV_DIR)

  ensure_dir("deps")

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
    # Build a 32-bit + 64-bit universal binary:
    "ARCHS=i386 x86_64", "ONLY_ACTIVE_ARCH=NO",
    "BUILD_DIR=out",
    "-project", LIB_UV_DIR + "/uv.xcodeproj",
    "-configuration", "Release",
    "-target", "All"
  ])


def build_libuv_linux(arch):
  # Set up the Makefile to build for the right architecture.
  args = ["python", "gyp_uv.py", "-f", "make"]
  if arch == "-32":
    args.append("-Dtarget_arch=ia32")
  elif arch == "-64":
    args.append("-Dtarget_arch=x64")

  run(args, cwd=LIB_UV_DIR)
  run(["make", "-C", "out", "BUILDTYPE=Release"], cwd=LIB_UV_DIR)


def build_libuv_windows():
  run(["cmd", "/c", "vcbuild.bat", "release"], cwd=LIB_UV_DIR)


def build_libuv(arch, out):
  if platform.system() == "Darwin":
    build_libuv_mac()
  elif platform.system() == "Linux":
    build_libuv_linux(arch)
  elif platform.system() == "Windows":
    build_libuv_windows()
  else:
    print("Unsupported platform: " + platform.system())
    sys.exit(1)

  # Copy the build library to the build directory for Mac and Linux where we
  # support building for multiple architectures.
  if platform.system() != "Windows":
    ensure_dir(os.path.dirname(out))
    shutil.copyfile(
      os.path.join(LIB_UV_DIR, "out", "Release", "libuv.a"), out)


def run(args, cwd=None):
  """Spawn a process to invoke [args] and mute its output."""
  try:
    subprocess.check_output(args, cwd=cwd, stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError as error:
    print(error.output)
    sys.exit(error.returncode)


def main():
  expect_usage(len(sys.argv) >= 2)

  if sys.argv[1] == "download":
    download_libuv()
  elif sys.argv[1] == "build":
    expect_usage(len(sys.argv) <= 3)
    arch = ""
    if len(sys.argv) == 3:
      arch = sys.argv[2]

    out = os.path.join("build", "libuv" + arch + ".a")

    build_libuv(arch, out)
  else:
    expect_usage(false)


def expect_usage(condition):
  if (condition): return

  print("Usage: libuv.py download")
  print("       libuv.py build [-32|-64]")
  sys.exit(1)


main()
