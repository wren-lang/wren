^title CLI Modules

The Wren CLI executable extends the built in language modules with its own,
which offer access to IO and other facilities for scripting.

The CLI modules are deeply tied to [libuv][], each other, and other internals
of the command-line app, so can't easily be separated out and pulled into host
applications that want to embed Wren. Scripts written for the CLI then,
are specific to the CLI unless another host implements the same API.

[libuv]: http://libuv.org

* [io](io)
* [os](os)
* [scheduler](scheduler)
* [timer](timer)
