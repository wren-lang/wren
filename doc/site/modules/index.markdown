^title Modules

Wren comes with built-in modules that are always available and not optional.
Every host that embeds Wren _should_ have these modules available.

## Core module

The core module is built directly into the VM and is implicitly
imported by every other module. You don't need to `import` anything to use it.
It contains objects and types for the language itself like [numbers][] and [strings][].

[numbers]: core/num.html
[strings]: core/string.html

* [core docs](core)

## Optional modules

Optional modules are available in the Wren project, but whether they are included is up to the host.
They are written in Wren and C, with no external dependencies, so including them in
your application is as easy as a simple compile flag.

Since they aren't *needed* by the VM itself to function, you can
disable some or all of them, so check if your host has them available.

So far there are a few optional modules:

* [meta docs](meta)
* [random docs](random)

