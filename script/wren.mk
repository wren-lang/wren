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
CLI_SOURCES  := $(wildcard src/cli/*.c)

MODULE_HEADERS   := $(wildcard src/module/*.h)
MODULE_SOURCES   := $(wildcard src/module/*.c)

VM_HEADERS   := $(wildcard src/vm/*.h)
VM_SOURCES   := $(wildcard src/vm/*.c)

TEST_SOURCES := $(shell find test/api -name '*.c')
BUILD_DIR := build

C_WARNINGS := -Wall -Wextra -Werror -Wno-unused-parameter
# Wren uses callbacks heavily, so -Wunused-parameter is too painful to enable.

# Mode configuration.
ifeq ($(MODE),debug)
	WREN := wrend
	C_OPTIONS += -O0 -DDEBUG -g
	BUILD_DIR := $(BUILD_DIR)/debug
else
	WREN += wren
	C_OPTIONS += -Os
	BUILD_DIR := $(BUILD_DIR)/release
endif

# Language configuration.
ifeq ($(LANG),cpp)
	WREN := $(WREN)-cpp
	C_OPTIONS += -std=c++98
	FILE_FLAG := -x c++
	BUILD_DIR := $(BUILD_DIR)-cpp
else
	C_OPTIONS += -std=c99
endif

# Architecture configuration.
ifeq ($(ARCH),32)
	C_OPTIONS += -m32
	WREN := $(WREN)-32
	BUILD_DIR := $(BUILD_DIR)-32
endif

ifeq ($(ARCH),64)
	C_OPTIONS += -m64
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
	C_OPTIONS += -fPIC
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
	LIBUV_BUILD := build
else
	SHARED_LIB_FLAGS := -Wl,-soname,$@.so
	SHARED_EXT := so
	LIBUV_BUILD := out

	# Enable pthread on gcc, since it isn't on by default.
	CFLAGS += -pthread
endif

CFLAGS := $(C_OPTIONS) $(C_WARNINGS)

CLI_OBJECTS    := $(addprefix $(BUILD_DIR)/cli/, $(notdir $(CLI_SOURCES:.c=.o)))
MODULE_OBJECTS := $(addprefix $(BUILD_DIR)/module/, $(notdir $(MODULE_SOURCES:.c=.o)))
VM_OBJECTS     := $(addprefix $(BUILD_DIR)/vm/, $(notdir $(VM_SOURCES:.c=.o)))
TEST_OBJECTS   := $(patsubst test/api/%.c, $(BUILD_DIR)/test/%.o, $(TEST_SOURCES))

LIBUV_DIR := build/libuv
LIBUV     := $(LIBUV_DIR)/$(LIBUV_BUILD)/Release/libuv.a

# Flags needed to compile source files for the CLI, including the modules and
# API tests.
CLI_FLAGS := -D_XOPEN_SOURCE=600 -Isrc/include -I$(LIBUV_DIR)/include \
             -Isrc/cli -Isrc/module

# Targets ---------------------------------------------------------------------

# Builds the VM libraries and CLI interpreter.
all: vm cli

# Builds just the VM libraries.
vm: lib/lib$(WREN).a lib/lib$(WREN).$(SHARED_EXT)

# Builds just the CLI interpreter.
cli: bin/$(WREN)

# Builds the API test executable.
test: $(BUILD_DIR)/test/$(WREN)

# Command-line interpreter.
bin/$(WREN): $(CLI_OBJECTS) $(MODULE_OBJECTS) $(VM_OBJECTS) $(LIBUV)
	@printf "%10s %-30s %s\n" $(CC) $@ "$(C_OPTIONS)"
	@mkdir -p bin
	@$(CC) $(CFLAGS) -L$(LIBUV_DIR)/$(LIBUV_BUILD)/Release -l uv -o $@ $^ -lm

# Static library.
lib/lib$(WREN).a: $(VM_OBJECTS)
	@printf "%10s %-30s %s\n" $(AR) $@ "rcu"
	@mkdir -p lib
	@$(AR) rcu $@ $^

# Shared library.
lib/lib$(WREN).$(SHARED_EXT): $(VM_OBJECTS)
	@printf "%10s %-30s %s\n" $(CC) $@ "$(C_OPTIONS) $(SHARED_LIB_FLAGS)"
	@mkdir -p lib
	@$(CC) $(CFLAGS) -shared $(SHARED_LIB_FLAGS) -o $@ $^

# Test executable.
$(BUILD_DIR)/test/$(WREN): $(TEST_OBJECTS) $(MODULE_OBJECTS) $(VM_OBJECTS) \
		$(BUILD_DIR)/cli/io.o $(BUILD_DIR)/cli/vm.o $(LIBUV)
	@printf "%10s %-30s %s\n" $(CC) $@ "$(C_OPTIONS)"
	@mkdir -p $(BUILD_DIR)/test
	@$(CC) $(CFLAGS) -L$(LIBUV_DIR)/$(LIBUV_BUILD)/Release -l uv -o $@ $^ -lm

# CLI object files.
$(BUILD_DIR)/cli/%.o: src/cli/%.c $(CLI_HEADERS) $(MODULE_HEADERS) $(VM_HEADERS)
	@printf "%10s %-30s %s\n" $(CC) $< "$(C_OPTIONS)"
	@mkdir -p $(BUILD_DIR)/cli
	@$(CC) -c $(CFLAGS) $(CLI_FLAGS) -o $@ $(FILE_FLAG) $<

# Module object files.
$(BUILD_DIR)/module/%.o: src/module/%.c $(CLI_HEADERS) $(MODULE_HEADERS) $(VM_HEADERS)
	@printf "%10s %-30s %s\n" $(CC) $< "$(C_OPTIONS)"
	@mkdir -p $(BUILD_DIR)/module
	@$(CC) -c $(CFLAGS) $(CLI_FLAGS) -o $@ $(FILE_FLAG) $<

# VM object files.
$(BUILD_DIR)/vm/%.o: src/vm/%.c $(VM_HEADERS)
	@printf "%10s %-30s %s\n" $(CC) $< "$(C_OPTIONS)"
	@mkdir -p $(BUILD_DIR)/vm
	@$(CC) -c $(CFLAGS) -Isrc/include -o $@ $(FILE_FLAG) $<

# Test object files.
$(BUILD_DIR)/test/%.o: test/api/%.c $(MODULE_HEADERS) $(VM_HEADERS)
	@printf "%10s %-30s %s\n" $(CC) $< "$(C_OPTIONS)"
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $(CLI_FLAGS) -o $@ $(FILE_FLAG) $<

.PHONY: all cli test vm
