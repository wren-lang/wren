# Top-level Makefile. This has targets for various utility things. To actually
# compile Wren itself, it invokes util/wren.mk for the various configurations
# that Wren can be built with.

# Allows one to enable verbose builds with VERBOSE=1
V := @
ifeq ($(VERBOSE),1)
	V :=
endif

# Executables are built to bin/. Libraries are built to lib/.

# A normal, optimized release build for the current CPU architecture.
# For convenience, also copies the interpreter to the top level.
release:
	$(V) $(MAKE) -f util/wren.mk
	$(V) cp bin/wren wren

# A debug build for the current architecture.
debug:
	$(V) $(MAKE) -f util/wren.mk MODE=debug

# A release build of just the VM, both shared and static libraries.
vm:
	$(V) $(MAKE) -f util/wren.mk vm

# A release build of the shared library for the VM.
shared:
	$(V) $(MAKE) -f util/wren.mk shared

# A release build of the shared library for the VM.
static:
	$(V) $(MAKE) -f util/wren.mk static

# Build all configurations.
all: debug release
	$(V) $(MAKE) -f util/wren.mk LANG=cpp
	$(V) $(MAKE) -f util/wren.mk MODE=debug LANG=cpp
	$(V) $(MAKE) -f util/wren.mk ARCH=32
	$(V) $(MAKE) -f util/wren.mk LANG=cpp ARCH=32
	$(V) $(MAKE) -f util/wren.mk MODE=debug ARCH=32
	$(V) $(MAKE) -f util/wren.mk MODE=debug LANG=cpp ARCH=32
	$(V) $(MAKE) -f util/wren.mk ARCH=64
	$(V) $(MAKE) -f util/wren.mk LANG=cpp ARCH=64
	$(V) $(MAKE) -f util/wren.mk MODE=debug ARCH=64
	$(V) $(MAKE) -f util/wren.mk MODE=debug LANG=cpp ARCH=64

# Travis uses these targets for continuous integration.
ci: ci_32 ci_64

ci_32:
	$(V) $(MAKE) -f util/wren.mk MODE=debug   LANG=c   ARCH=32 vm cli api_test
	$(V) ./util/test.py --suffix=d-32 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=debug   LANG=cpp ARCH=32 vm cli api_test
	$(V) ./util/test.py --suffix=d-cpp-32 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=release LANG=c   ARCH=32 vm cli api_test
	$(V) ./util/test.py --suffix=-32 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=release LANG=cpp ARCH=32 vm cli api_test
	$(V) ./util/test.py --suffix=-cpp-32 $(suite)

ci_64:
	$(V) $(MAKE) -f util/wren.mk MODE=debug   LANG=c   ARCH=64 vm cli api_test
	$(V) ./util/test.py --suffix=d-64 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=debug   LANG=cpp ARCH=64 vm cli api_test
	$(V) ./util/test.py --suffix=d-cpp-64 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=release LANG=c   ARCH=64 vm cli api_test
	$(V) ./util/test.py --suffix=-64 $(suite)
	$(V) $(MAKE) -f util/wren.mk MODE=release LANG=cpp ARCH=64 vm cli api_test
	$(V) ./util/test.py --suffix=-cpp-64 $(suite)

# Remove all build outputs and intermediate files. Does not remove downloaded
# dependencies. Use cleanall for that.
clean:
	$(V) rm -rf bin
	$(V) rm -rf build
	$(V) rm -rf lib

# Run the tests against the debug build of Wren.
test: api_test debug
	$(V) ./util/test.py $(suite)

benchmark: release
	$(V) $(MAKE) -f util/wren.mk api_test
	$(V) ./util/benchmark.py -l wren $(suite)

benchmark_baseline: release
	$(V) $(MAKE) -f util/wren.mk api_test
	$(V) ./util/benchmark.py --generate-baseline

unit_test:
	$(V) $(MAKE) -f util/wren.mk MODE=debug unit_test
	$(V) ./build/debug/test/unit_wrend

# Build API tests.
api_test:
	$(V) $(MAKE) -f util/wren.mk MODE=debug api_test

# Generate the Wren site.
docs:
	mkdir -p build
	$(V) ./util/generate_docs.py

# Continuously generate and serve the Wren site.
servedocs:
	$(V) ./util/generate_docs.py --serve

# Continuously generate the Wren site.
watchdocs:
	$(V) ./util/generate_docs.py --watch

# Build the docs and copy them to a local "gh-pages" directory.
gh-pages: docs
	$(V) cp -r build/docs/. build/gh-pages

# Build amalgamation of all Wren library files.
amalgamation: src/include/wren.h src/vm/*.h src/vm/*.c
	./util/generate_amalgamation.py > build/wren.c

.PHONY: all amalgamation api_test benchmark builtin clean debug docs gh-pages release test vm watchdocs ci ci_32 ci_64
