# Compiler flags.
CFLAGS = -std=c99 -Wall -Werror
# TODO: Add -Wextra.
DEBUG_CFLAGS = -O0 -DDEBUG
RELEASE_CFLAGS = -Os

# Files.
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(SOURCES:.c=.o)

DEBUG_OBJECTS = $(addprefix build/debug/, $(notdir $(OBJECTS)))
RELEASE_OBJECTS = $(addprefix build/release/, $(notdir $(OBJECTS)))

.PHONY: all clean test builtin docs watchdocs

all: release

clean:
	rm -rf build wren wrend

prep:
	mkdir -p build/debug build/release

# Debug build.
debug: prep wrend

# Debug command-line interpreter.
wrend: $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -lm -o wrend $^

# Debug object files.
build/debug/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $<

# Release build.
release: prep wren

# Release command-line interpreter.
wren: $(RELEASE_OBJECTS)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -lm -o wren $^

# Release object files.
build/release/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $<

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
