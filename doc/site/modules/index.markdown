^title Modules

Wren comes with two kinds of modules, the core module (built-in),
and a few optional modules that the host embedding Wren can enable.

## Core module

The core module is built directly into the VM and is implicitly
imported by every other module. You don't need to `import` anything to use it.
It contains objects and types for the language itself like [numbers][] and [strings][].

Because Wren is designed for [embedding in applications][embedding], its core
module is minimal and is focused on working with objects within Wren. For
stuff like file IO, graphics, etc., it is up to the host application to provide
interfaces for this.

[numbers]: core/num.html
[strings]: core/string.html

## Optional modules

Optional modules are available in the Wren project, but whether they are included is up to the host.
They are written in Wren and C, with no external dependencies, so including them in
your application is as easy as a simple compile flag.

Since they aren't *needed* by the VM itself to function, you can
disable some or all of them, so check if your host has them available.

So far there are a few optional modules:

* [meta docs](meta)
* [random docs](random)

