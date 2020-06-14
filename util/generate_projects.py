#!/usr/bin/env python

import sys
from os import getenv, path
from subprocess import PIPE, run

PREMAKE_DIR = path.join(path.dirname(__file__), "../projects/premake")


def has(prog):
    return run(["command", "-v", prog], stdout=PIPE, stderr=PIPE).returncode == 0


def run_premake(action, os):
    run([premake, action, os], cwd=PREMAKE_DIR)


premake = getenv("PREMAKE", "premake5")
if not has(premake):
    print("error: {} is not found", premake, file=sys.stderr)
    exit(1)

run_premake("gmake2", "bsd")
run_premake("gmake2", "linux")
run_premake("gmake2", "macosx")
run_premake("vs2017", "windows")
run_premake("vs2019", "windows")
run_premake("xcode4", "macosx")
