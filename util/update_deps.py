#!/usr/bin/env python

# Downloads GYP and libuv into deps/.
#
# Run this manually to update the vendored copies of GYP and libuv that are
# committed in the Wren repo.

from __future__ import print_function

import sys, os

from util import clean_dir, remove_dir, try_remove_dir, replace_in_file, run

LIB_UV_VERSION = "v1.27.0"
LIB_UV_DIR = "deps/libuv"


def main(args):
  # Delete it if already there so we ensure we get the correct version if the
  # version number in this script changes.
  clean_dir("deps")

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

  # We don't need all of libuv and gyp's various support files.
  print("Deleting unneeded files...")
  try_remove_dir("deps/libuv/build/gyp/buildbot")
  try_remove_dir("deps/libuv/build/gyp/infra")
  try_remove_dir("deps/libuv/build/gyp/samples")
  try_remove_dir("deps/libuv/build/gyp/test")
  try_remove_dir("deps/libuv/build/gyp/tools")
  try_remove_dir("deps/libuv/docs")
  try_remove_dir("deps/libuv/img")
  try_remove_dir("deps/libuv/samples")

  # We are going to commit libuv and GYP in the main Wren repo, so we don't
  # want them to be their own repos.
  remove_dir("deps/libuv/.git")
  remove_dir("deps/libuv/build/gyp/.git")

  # Libuv's .gitignore ignores GYP, but we want to commit it.
  if os.path.isfile("deps/libuv/.gitignore"):
    with open("deps/libuv/.gitignore", "a") as libuv_ignore:
      libuv_ignore.write("!build/gyp")


main(sys.argv)
