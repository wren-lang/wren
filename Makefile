.PHONY: all clean test docs corelib

all:
	gcc -Iinclude src/*.c -owren
	mkdir -p build/Release
	mv wren build/Release

clean:
	rm -rf build

test:
	@./runtests

docs:
	@./make_docs

corelib:
	@./make_corelib
