^title Modules

Because Wren can be used both as an embedded scripting language, and as a
general purpose programming language run from the command line, the definition
of a "built-in" module is a little complicated. They are organized into three
categories:

## Core

There is one core module. It is built directly into the VM and is implicitly
imported by every other module. It contains the classes for the objects built
directly into the language itself: [numbers][], [strings][], etc.

[numbers]: core/num.html
[strings]: core/string.html

The core module is always available and can't be removed.

* [core](core)

## Optional

Optional modules are available in the command line Wren interpreter. When you
embed Wren in your own host application, you can also include them too. They are
written in Wren and C, but have no external dependencies, so including them in
your application doesn't force you to bring in any other third-party code.

At the same time, they aren't *needed* by the VM itself to function, so you can
disable some or all of them if you want to keep your app as small and
constrained as possible.

There are a couple of optional modules:

* [meta](meta)
* [random](random)

## CLI

The CLI modules are only available in the standalone command-line Wren
interpreter. They are deeply tied to [libuv][], each other, and other internals
of the command-line app, so can't be separated out and pulled into host
applications that want to embed Wren.

[libuv]: http://libuv.org

* [io](io)
* [os](os)
* [scheduler](scheduler)
* [timer](timer)
