// Handle empty list.
System.print([].toString)             // expect: []

// Does not quote strings.
System.print([1, "2", true].toString) // expect: [1, 2, true]

// Nested lists.
System.print([1, [2, [3], 4], 5]) // expect: [1, [2, [3], 4], 5]

// Calls toString on elements.
class Foo {
  construct new() {}
  toString { "Foo.toString" }
}

System.print([1, Foo.new(), 2]) // expect: [1, Foo.toString, 2]

// TODO: Handle lists that contain themselves.
