#!/usr/bin/env python

# Runs GYP to generate the right project then uses that to build libuv.

from __future__ import print_function

import os
import platform
import shutil
import sys

from util import ensure_dir, python2_binary, run

LIB_UV_VERSION = "v1.35.0"
LIB_UV_DIR = "deps/libuv"


def build_libuv_mac():
  args = [python2_binary(), "gyp_uv.py", "-f", "make", "-Dtarget_arch=x86_64"]
  run(args, cwd=LIB_UV_DIR)
  # TODO: Support debug builds too.
  run(["make", "-C", "out", "BUILDTYPE=Release", "libuv"], cwd=LIB_UV_DIR)


def build_libuv_linux(arch):
  # Set up the Makefile to build for the right architecture.
  args = [python2_binary(), "gyp_uv.py", "-f", "make"]
  if arch == "-32":
    args.append("-Dtarget_arch=ia32")
  elif arch == "-64":
    args.append("-Dtarget_arch=x64")

  run(args, cwd=LIB_UV_DIR)
  run(["make", "-C", "out", "BUILDTYPE=Release", "libuv"], cwd=LIB_UV_DIR)


def build_libuv_windows(arch):
  args = ["cmd", "/c", "vcbuild.bat", "release", "vs2017"]
  if arch == "-32":
    args.append("x86")
  elif arch == "-64":
    args.append("x64")
  run(args, cwd=LIB_UV_DIR)


def build_libuv(arch, out):
  if platform.system() == "Darwin":
    build_libuv_mac()
  elif platform.system() == "Linux":
    build_libuv_linux(arch)
  elif platform.system() == "Windows":
    build_libuv_windows(arch)
  else:
    print("Unsupported platform: " + platform.system())
    sys.exit(1)

  # Copy the build library to the build directory for Mac and Linux where we
  # support building for multiple architectures.
  if platform.system() != "Windows":
    ensure_dir(os.path.dirname(out))
    shutil.copyfile(
      os.path.join(LIB_UV_DIR, "out", "Release", "libuv.a"), out)


def main(args):
  expect_usage(len(args) >= 1 and len(args) <= 2)

  arch = "" if len(args) < 2 else args[1]
  out = os.path.join("build", "libuv" + arch + ".a")

  build_libuv(arch, out)


def expect_usage(condition):
  if (condition): return

  print("Usage: build_libuv.py [-32|-64]")
  sys.exit(1)


main(sys.argv)
