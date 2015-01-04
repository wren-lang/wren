# Compiler flags.
CFLAGS := -std=c99 -Wall -Werror
# TODO: Add -Wextra.

# Files.
SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)
OBJECTS := $(SOURCES:.c=.o)

DEBUG_OBJECTS := $(addprefix build/debug/, $(notdir $(OBJECTS)))
RELEASE_OBJECTS := $(addprefix build/release/, $(notdir $(OBJECTS)))

.PHONY: all clean test builtin docs watchdocs

all: release

clean:
	@rm -rf build wren wrend

prep:
	@mkdir -p build/debug build/release

# Debug build.
debug: prep wrend

# Debug command-line interpreter.
wrend: CFLAGS += -O0 -DDEBUG

wrend: $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) -Iinclude -o wrend $^ -lm

# Debug object files.
build/debug/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) -Iinclude -o $@ $<

# Release build.
release: prep wren

# Release command-line interpreter.
wren: CFLAGS += -Os

wren: $(RELEASE_OBJECTS)
	$(CC) $(CFLAGS) -Iinclude -o wren $^ -lm

# Release object files.
build/release/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) -Iinclude -o $@ $<

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
