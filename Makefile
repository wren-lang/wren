AR = ar rcu
# Compiler flags.
CFLAGS = -std=c99 -Wall -Werror
# TODO: Add -Wextra.
DEBUG_CFLAGS = -O0 -DDEBUG
RELEASE_CFLAGS = -Os

# Files.
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(SOURCES:.c=.o)

# Don't include main.c in the shared library.
DEBUG_OBJECTS = $(subst build/debug/main.o,,$(addprefix build/debug/, $(notdir $(OBJECTS))))
RELEASE_OBJECTS = $(subst build/release/main.o,,$(addprefix build/release/, $(notdir $(OBJECTS))))


.PHONY: all clean test builtin docs watchdocs

all: release

clean:
	@rm -rf build wren wrend libwren.a libwrend.a

prep:
	@mkdir -p build/debug build/release

# Debug build.
debug: prep wrend

# Debug shared lib
libwrend.a: $(DEBUG_OBJECTS)
	$(AR) $@ $^

# Debug command-line interpreter.
wrend: build/debug/main.o libwrend.a
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o wrend $^ -lm

# Debug object files.
build/debug/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c -fPIC $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $<

# Release build.
release: prep wren

# Release shared lib
libwren.a: $(RELEASE_OBJECTS)
	$(AR) $@ $^

# Release command-line interpreter.
wren: build/release/main.o libwren.a
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o wren $^ -lm

# Release object files.
build/release/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c -fPIC $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $<

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

# Run the benchmark suite.
benchmark: wren
	@cd benchmark && ./run_bench || cd -

# Run the metrics script.
metrics:
	@./script/metrics.py
