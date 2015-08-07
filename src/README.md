This contains the Wren source code. It is organized like so:

* `cli`: the source code for the command line interface. This is a custom
  exectuable that embeds the VM in itself. Code here handles reading
  command-line, running the REPL, loading modules from disc, etc.

* `include`: the public header directory for the VM. If you are embedding the
  VM in your own application, you will add this to your include path.

* `module`: the source code for the built-in modules that come with the CLI.
  These modules are written in a mixture of C and Wren and generally use
  [libuv][] to implement their underlying functionality.

* `vm`: the source code for the Wren VM itself. Unlike code in `cli` and
  `module`, this has no dependencies on libuv. If you are embedding the VM in
  your own application from source, you will compile the files here into your
  app.

[libuv]: http://libuv.org/
