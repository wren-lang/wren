AR := ar rcu
# Compiler flags.
CFLAGS := -std=c99 -Wall -Werror
# TODO: Add -Wextra.
CPPFLAGS := -std=c++98 -Wall -Werror
DEBUG_CFLAGS := -O0 -DDEBUG -g
RELEASE_CFLAGS := -Os

# Detect the OS.
TARGET_OS := $(lastword $(subst -, ,$(shell $(CC) -dumpmachine)))

# Don't add -fPIC on Windows since it generates a warning which gets promoted
# to an error by -Werror.
ifeq      ($(TARGET_OS),mingw32)
else ifeq ($(TARGET_OS),cygwin)
	# Do nothing.
else
	CFLAGS += -fPIC
	CPPFLAGS += -fPIC
endif

# Clang on Mac OS X has different flags and a different extension to build a
# shared library.
ifneq (,$(findstring darwin,$(TARGET_OS)))
	SHARED_LIB_FLAGS =
	SHARED_EXT = dylib
else
	SHARED_LIB_FLAGS = -Wl,-soname,$@.so
	SHARED_EXT = so
endif

# Files.
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
OBJECTS := $(SOURCES:.c=.o)

# Don't include main.c in the libraries.
DEBUG_OBJECTS := $(addprefix build/debug/, $(notdir $(OBJECTS)))
RELEASE_OBJECTS := $(addprefix build/release/, $(notdir $(OBJECTS)))
RELEASE_CPP_OBJECTS := $(addprefix build/release-cpp/, $(notdir $(OBJECTS)))

DEBUG_LIB_OBJECTS := $(subst build/debug/main.o,,$(DEBUG_OBJECTS))
RELEASE_LIB_OBJECTS := $(subst build/release/main.o,,$(RELEASE_OBJECTS))
RELEASE_CPP_LIB_OBJECTS := $(subst build/release-cpp/main.o,,$(RELEASE_CPP_OBJECTS))

.PHONY: all clean test builtin docs watchdocs prep

all: release

clean:
	@rm -rf build wren wrend libwren libwrend

prep:
	@mkdir -p build/debug build/release build/release-cpp

# Debug build.
debug: prep wrend libwrend

# Debug static and shared libraries.
libwrend: $(DEBUG_LIB_OBJECTS)
	$(AR) $@.a $^
	$(CC) $(DEBUG_CFLAGS) -shared $(SHARED_LIB_FLAGS) -o $@.$(SHARED_EXT) $^

# Debug command-line interpreter.
wrend: $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $^ -lm

# Debug object files.
build/debug/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $<

# Release build.
release: prep wren libwren

# Release static and shared libraries.
libwren: $(RELEASE_LIB_OBJECTS)
	$(AR) $@.a $^
	$(CC) $(RELEASE_CFLAGS) -shared $(SHARED_LIB_FLAGS) -o $@.$(SHARED_EXT) $^

# Release command-line interpreter.
wren: $(RELEASE_OBJECTS)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $^ -lm

# Release object files.
build/release/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $<

# Release C++ build.
release-cpp: prep wren-cpp libwren-cpp

# Release C++ static library.
libwren-cpp: $(RELEASE_CPP_LIB_OBJECTS)
	$(AR) $@.a $^

# Release C++ command-line interpreter.
wren-cpp: $(RELEASE_CPP_OBJECTS)
	$(CC) $(CPPFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $^ -lm

# Release C++ object files.
build/release-cpp/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CPPFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ -x c++ $<

# Run the tests against the debug build of Wren.
test: debug
	@./script/test.py $(suite)

# Take the contents of the scripts under builtin/ and copy them into their
# respective wren_<name>.c source files.
builtin:
	@./script/generate_builtins.py

# Generate the Wren site.
docs:
	@./script/generate_docs.py

# Continuously generate the Wren site.
watchdocs:
	@./script/generate_docs.py --watch

# Build the docs and copy them to a local "gh-pages" directory.
gh-pages: docs
	cp -r build/docs/. gh-pages
