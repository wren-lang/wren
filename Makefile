AR = ar rcu
# Compiler flags.
CFLAGS = -std=c99 -Wall -Werror
# TODO: Add -Wextra.
CPPFLAGS = -std=c++98 -Wall -Werror
DEBUG_CFLAGS = -O0 -DDEBUG -g
RELEASE_CFLAGS = -Os

# Files.
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(SOURCES:.c=.o)

# Don't include main.c in the shared library.
DEBUG_OBJECTS = $(addprefix build/debug/, $(notdir $(OBJECTS)))
RELEASE_OBJECTS = $(addprefix build/release/, $(notdir $(OBJECTS)))
RELEASE_CPP_OBJECTS = $(addprefix build/release-cpp/, $(notdir $(OBJECTS)))

DEBUG_LIB_OBJECTS = $(subst build/debug/main.o,,$(DEBUG_OBJECTS))
RELEASE_LIB_OBJECTS = $(subst build/release/main.o,,$(RELEASE_OBJECTS))
RELEASE_CPP_LIB_OBJECTS = $(subst build/release-cpp/main.o,,$(RELEASE_CPP_OBJECTS))

.PHONY: all clean test builtin docs watchdocs

all: release

clean:
	@rm -rf build wren wrend libwren.a libwrend.a

prep:
	@mkdir -p build/debug build/release build/release-cpp

# Debug build.
debug: prep wrend libwrend.a

# Debug shared library.
libwrend.a: $(DEBUG_LIB_OBJECTS)
	$(AR) $@ $^

# Debug command-line interpreter.
wrend: $(DEBUG_OBJECTS)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $^ -lm

# Debug object files.
build/debug/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c -fPIC $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -o $@ $<

# Release build.
release: prep wren libwren.a

# Release shared library.
libwren.a: $(RELEASE_LIB_OBJECTS)
	$(AR) $@ $^

# Release command-line interpreter.
wren: $(RELEASE_OBJECTS)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $^ -lm

# Release object files.
build/release/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c -fPIC $(CFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $<

# Release C++ build.
release-cpp: prep wren-cpp libwren-cpp.a

# Release C++ shared lib
libwren-cpp.a: $(RELEASE_CPP_LIB_OBJECTS)
	$(AR) $@ $^

# Release C++ command-line interpreter.
wren-cpp: $(RELEASE_CPP_OBJECTS)
	$(CC) $(CPPFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ $^ -lm

# Release C++ object files.
build/release-cpp/%.o: src/%.c include/wren.h $(HEADERS)
	$(CC) -c -fPIC $(CPPFLAGS) $(RELEASE_CFLAGS) -Iinclude -o $@ -x c++ $<

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
