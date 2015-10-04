The Wren scripts in this directory get converted to C string literals into files
with a `.wren.inc` extension. Those are then `#include`d into their respective
`.c` files so that the interpreter can load them directly without having to do
any file IO.

The script that does this translation is `util/wren_to_c_string.py`.

When any of the ".wren" files in here are changed, the Makefile automatically
updates the generated C headers.
