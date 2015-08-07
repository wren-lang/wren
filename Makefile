# Top-level Makefile. This has targets for various utility things. To actually
# compile Wren itself, it invokes script/wren.mk for the various configurations
# that Wren can be built with.

# Executables are built to bin/. Libraries are built to lib/.

LIBUV := build/libuv/build/Release/libuv.a

# A normal, optimized release build for the current CPU architecture.
release: $(LIBUV)
	@ $(MAKE) -f script/wren.mk
	@ cp bin/wren wren # For convenience, copy the interpreter to the top level.

# A debug build for the current architecture.
debug: $(LIBUV)
	@ $(MAKE) -f script/wren.mk MODE=debug

# A release build of just the VM. Does not require libuv.
vm:
	@ $(MAKE) -f script/wren.mk vm

# Build all configurations.
all: debug release
	@ $(MAKE) -f script/wren.mk LANG=cpp
	@ $(MAKE) -f script/wren.mk MODE=debug LANG=cpp
	@ $(MAKE) -f script/wren.mk ARCH=32
	@ $(MAKE) -f script/wren.mk LANG=cpp ARCH=32
	@ $(MAKE) -f script/wren.mk MODE=debug ARCH=32
	@ $(MAKE) -f script/wren.mk MODE=debug LANG=cpp ARCH=32
	@ $(MAKE) -f script/wren.mk ARCH=64
	@ $(MAKE) -f script/wren.mk LANG=cpp ARCH=64
	@ $(MAKE) -f script/wren.mk MODE=debug ARCH=64
	@ $(MAKE) -f script/wren.mk MODE=debug LANG=cpp ARCH=64

# Download and build libuv to a static library.
$(LIBUV): script/libuv.py
	@ ./script/libuv.py

# Remove all build outputs and intermediate files.
clean:
	@ rm -rf bin
	@ rm -rf build
	@ rm -rf lib

# Run the tests against the debug build of Wren.
test: debug
	@ $(MAKE) -f script/wren.mk MODE=debug test
	@ ./script/test.py $(suite)

# Take the contents of the scripts under builtin/ and copy them into their
# respective wren_<name>.c source files.
builtin:
	@ ./script/generate_builtins.py

# Generate the Wren site.
docs:
	@ ./script/generate_docs.py

# Continuously generate the Wren site.
watchdocs:
	@ ./script/generate_docs.py --watch

# Build the docs and copy them to a local "gh-pages" directory.
gh-pages: docs
	@ cp -r build/docs/. build/gh-pages

# Build amalgamation of all Wren library files.
amalgamation: src/include/wren.h src/vm/*.h src/vm/*.c
	./script/generate_amalgamation.py > build/wren.c

.PHONY: all amalgamation builtin clean debug docs gh-pages release test vm watchdocs
