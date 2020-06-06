#!/bin/bash
set -e

# This build script only builds mac or linux right now, for CI.
WREN_WD="projects/make"
if [ -n "$WREN_TARGET_MAC" ]; then
  WREN_WD="projects/make.mac"
fi

WREN_PY=${WREN_PY_BINARY:-python3}

echo "using working directory '$WREN_WD' ..."
echo "using python binary '$WREN_PY' ..."

make -C $WREN_WD config=debug_64bit-no-nan-tagging
$WREN_PY ./util/test.py --suffix=_d

make -C $WREN_WD config=debug_64bit
$WREN_PY ./util/test.py --suffix=_d

make -C $WREN_WD config=release_64bit-no-nan-tagging
$WREN_PY ./util/test.py

make -C $WREN_WD config=release_64bit
$WREN_PY ./util/test.py
