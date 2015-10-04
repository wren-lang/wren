# Top-level Makefile. This has targets for various utility things. To actually
# compile Wren itself, it invokes util/wren.mk for the various configurations
# that Wren can be built with.

# Executables are built to bin/. Libraries are built to lib/.

# A normal, optimized release build for the current CPU architecture.
release:
	@ $(MAKE) -f util/wren.mk
	@ cp bin/wren wren # For convenience, copy the interpreter to the top level.

# A debug build for the current architecture.
debug:
	@ $(MAKE) -f util/wren.mk MODE=debug

# A release build of just the VM.
vm:
	@ $(MAKE) -f util/wren.mk vm

# Build all configurations.
all: debug release
	@ $(MAKE) -f util/wren.mk LANG=cpp
	@ $(MAKE) -f util/wren.mk MODE=debug LANG=cpp
	@ $(MAKE) -f util/wren.mk ARCH=32
	@ $(MAKE) -f util/wren.mk LANG=cpp ARCH=32
	@ $(MAKE) -f util/wren.mk MODE=debug ARCH=32
	@ $(MAKE) -f util/wren.mk MODE=debug LANG=cpp ARCH=32
	@ $(MAKE) -f util/wren.mk ARCH=64
	@ $(MAKE) -f util/wren.mk LANG=cpp ARCH=64
	@ $(MAKE) -f util/wren.mk MODE=debug ARCH=64
	@ $(MAKE) -f util/wren.mk MODE=debug LANG=cpp ARCH=64

# Remove all build outputs and intermediate files. Does not remove downloaded
# dependencies. Use cleanall for that.
clean:
	@ rm -rf bin
	@ rm -rf build
	@ rm -rf lib

# Remove all build outputs, intermediate files, and downloaded dependencies.
cleanall: clean
	@ rm -rf deps

# Run the tests against the debug build of Wren.
test: debug
	@ $(MAKE) -f util/wren.mk MODE=debug test
	@ ./util/test.py $(suite)

# Generate the Wren site.
docs:
	@ ./util/generate_docs.py

# Continuously generate the Wren site.
watchdocs:
	@ ./util/generate_docs.py --watch

# Build the docs and copy them to a local "gh-pages" directory.
gh-pages: docs
	@ cp -r build/docs/. build/gh-pages

# Build amalgamation of all Wren library files.
amalgamation: src/include/wren.h src/vm/*.h src/vm/*.c
	./util/generate_amalgamation.py > build/wren.c

.PHONY: all amalgamation builtin clean debug docs gh-pages release test vm watchdocs
