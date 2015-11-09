^title Module "core"

Because Wren is designed for [embedding in applications][embedding], its core
module is minimal and is focused on working with objects within Wren. For
stuff like file IO, graphics, etc., it is up to the host application to provide
interfaces for this.

All Wren source files automatically have access to the following classes:

* [Bool](bool.html)
* [Class](class.html)
* [Fiber](fiber.html)
* [Fn](fn.html)
* [List](list.html)
* [Map](map.html)
* [Null](null.html)
* [Num](num.html)
* [Object](object.html)
* [Range](range.html)
* [Sequence](sequence.html)
* [String](string.html)
* [System](system.html)

[embedding]: ../../embedding-api.html
