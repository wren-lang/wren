^title Wren CLI

---

## What is it?

**The Wren Command-Line Interface** is a tool you can run which gives you a way to run Wren code, and
    also includes modules for talking to the operating system&mdash;file IO,
    networking, stuff like that. It depends on [libuv][] for that
    functionality.

Wren as a language is intentionally designed to be minimal.   
That includes the built in language features, the standard library and the VM itself.

In order to access files, networks and other IO, you'd need to make a tool _using_ the language VM. 
That's what the CLI project is! It is not bundled as part of the wren project,
instead it is its own project as a standalone tool you can run.
It exposes its own standard library and modules that may be of interest
if looking for a general purpose single binary scriptable tool.

Wren CLI is a work in progress, and contributions are welcome to make it more useful over time.

## Why does it exist?

- It's fun to make things.
- It's always a good idea to test the language you're making!
- Interest was expressed in a scriptable tool using the Wren language.
- It's helpful for others to learn from, since it is a real world usage example showing several concepts.

[libuv]: http://libuv.org/
