// Handle empty list.
IO.print([].toString)             // expect: []

// Does not quote strings.
IO.print([1, "2", true].toString) // expect: [1, 2, true]

// Nested lists.
IO.print([1, [2, [3], 4], 5]) // expect: [1, [2, [3], 4], 5]

// Calls toString on elements.
class Foo {
  toString { "Foo.toString" }
}

IO.print([1, new Foo, 2]) // expect: [1, Foo.toString, 2]

// TODO: Handle lists that contain themselves.
