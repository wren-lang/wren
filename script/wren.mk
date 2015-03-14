# Makefile for building a single configuration of Wren. It allows the
# following variables to be passed to it:
#
# MODE - The build mode, "debug" or "release".
#				 If omitted, defaults to "release".
# LANG - The language, "c" or "cpp".
# 		   If omitted, defaults to "c".
# ARCH - The processor architecture, "32", "64", or nothing, which indicates
#				 the compiler's default.
#        If omitted, defaults to the compiler's default.
#
# It builds a static library, shared library, and command-line interpreter for
# the given configuration. Libraries are built to "lib", and the interpreter
# is built to "bin".
#
# The output file is initially "wren". If in debug mode, "d" is appended to it.
# If the language is "cpp", then "-cpp" is appended to that. If the
# architecture is not the default then "-32" or "-64" is appended to that.
# Then, for the libraries, the correct extension is added.

# Files.
CLI_HEADERS  := $(wildcard src/cli/*.h)
VM_HEADERS   := $(wildcard src/vm/*.h)
CLI_SOURCES  := $(wildcard src/cli/*.c)
VM_SOURCES   := $(wildcard src/vm/*.c)
BUILD_DIR := build

CFLAGS := -Wall -Werror -Wsign-compare -Wtype-limits -Wuninitialized
# TODO: Add -Wextra.

# Mode configuration.
ifeq ($(MODE),debug)
	WREN := wrend
	CFLAGS += -O0 -DDEBUG -g
	BUILD_DIR := $(BUILD_DIR)/debug
else
	WREN += wren
	CFLAGS += -Os
	BUILD_DIR := $(BUILD_DIR)/release
endif

# Language configuration.
ifeq ($(LANG),cpp)
	WREN := $(WREN)-cpp
	CFLAGS += -std=c++98
	FILE_FLAG := -x c++
	BUILD_DIR := $(BUILD_DIR)-cpp
else
	CFLAGS += -std=c99
endif

# Architecture configuration.
ifeq ($(ARCH),32)
	CFLAGS += -m32
	WREN := $(WREN)-32
	BUILD_DIR := $(BUILD_DIR)-32
endif

ifeq ($(ARCH),64)
	CFLAGS += -m64
	WREN := $(WREN)-64
	BUILD_DIR := $(BUILD_DIR)-64
endif

# Some platform-specific workarounds. Note that we use "gcc" explicitly in the
# call to get the machine name because one of these workarounds deals with $(CC)
# itself not working.
OS := $(lastword $(subst -, ,$(shell gcc -dumpmachine)))

# Don't add -fPIC on Windows since it generates a warning which gets promoted
# to an error by -Werror.
ifeq      ($(OS),mingw32)
else ifeq ($(OS),cygwin)
	# Do nothing.
else
	CFLAGS += -fPIC
endif

# MinGW--or at least some versions of it--default CC to "cc" but then don't
# provide an executable named "cc". Manually point to "gcc" instead.
ifeq ($(OS),mingw32)
	CC = GCC
endif

# Clang on Mac OS X has different flags and a different extension to build a
# shared library.
ifneq (,$(findstring darwin,$(OS)))
	SHARED_EXT := dylib
else
	SHARED_LIB_FLAGS := -Wl,-soname,$@.so
	SHARED_EXT := so
endif

CLI_OBJECTS := $(addprefix $(BUILD_DIR)/cli/, $(notdir $(CLI_SOURCES:.c=.o)))
VM_OBJECTS := $(addprefix $(BUILD_DIR)/vm/, $(notdir $(VM_SOURCES:.c=.o)))

# Targets ---------------------------------------------------------------------

all: prep bin/$(WREN) lib/lib$(WREN).a lib/lib$(WREN).$(SHARED_EXT)

prep:
	@mkdir -p bin lib $(BUILD_DIR)/cli $(BUILD_DIR)/vm

# Command-line interpreter.
bin/$(WREN): $(CLI_OBJECTS) $(VM_OBJECTS)
	$(CC) $(CFLAGS) -Isrc/include -o $@ $^ -lm

# Static library.
lib/lib$(WREN).a: $(VM_OBJECTS)
	$(AR) rcu $@ $^

# Shared library.
lib/lib$(WREN).$(SHARED_EXT): $(VM_OBJECTS)
	$(CC) $(CFLAGS) -shared $(SHARED_LIB_FLAGS) -o $@ $^

# CLI object files.
$(BUILD_DIR)/cli/%.o: src/cli/%.c $(CLI_HEADERS) $(VM_HEADERS)
	$(CC) -c $(CFLAGS) -Isrc/include -o $@ $(FILE_FLAG) $<

# VM object files.
$(BUILD_DIR)/vm/%.o: src/vm/%.c $(VM_HEADERS)
	$(CC) -c $(CFLAGS) -Isrc/include -o $@ $(FILE_FLAG) $<
