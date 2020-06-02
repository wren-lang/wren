#!/bin/bash
set -e

# This build script only builds mac or linux right now, for CI.
WREN_WD="projects/make"
if [ -n "$WREN_TARGET_MAC" ]
then
  WREN_WD="projects/make.mac"
fi

WREN_PY="python3"
if [ -n "$WREN_PY_BINARY" ]
then
  WREN_PY="$WREN_PY_BINARY"
fi

echo "using working directory '$WREN_WD' ..."
echo "using python binary '$WREN_PY' ..."

cd "$WREN_WD" && make config=debug_64bit-no-nan-tagging
cd ../../ && $WREN_PY ./util/test.py --suffix=_d

cd "$WREN_WD" && make config=debug_64bit
cd ../../ && $WREN_PY ./util/test.py --suffix=_d

cd "$WREN_WD" && make config=release_64bit-no-nan-tagging
cd ../../ && $WREN_PY ./util/test.py

cd "$WREN_WD" && make config=release_64bit
cd ../../ && $WREN_PY ./util/test.py
