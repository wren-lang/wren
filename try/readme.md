# try wren implementation

This is the code to build the https://wren.io/try wasm component.

### How to build

- Install emscripten sdk from https://emscripten.org/
- Make the emsdk available to your terminal/PATH 
	- for example:
    - `source ~/dev/emsdk/emsdk_env.sh`
- Run the emmake command to build
    - `emmake make`

That should be all. This builds a js + wasm file for the page.

### How does the page work?

The page is at `doc/site/try/template.html`.

It loads `wren_try.js` which loads `wren_try.wasm`.
The page uses emscripten API to call the `wren_compile` C function, found in `main.try.c`.
The page hooks up `printf` logging to the console for display.

### Notes

- The binaries land in `bin/wren_try.wasm` and `bin/wren_try.js` when building
- The default html output from emsripten is not used, `doc/site/try/template.html` is
- The wren_try.js and wren_try.wasm files are copied to `doc/site/static`
- The make project is a modified version of `projects/make`
- The code relies on code in `test/`
