// Handle empty list.
IO.print([].join(",") == "") // expect: true

// Handle a simple list with an empty delimeter.
IO.print([1, 2, 3].join("")) // expect: 123

// Handle a simple list with no separator.
IO.print([1, 2, 3].join) // expect: 123

// Does not quote strings.
IO.print([1, "2", true].join(",")) // expect: 1,2,true

// Nested lists.
IO.print([1, [2, [3], 4], 5].join(",")) // expect: 1,[2, [3], 4],5

// Calls toString on elements.
class Foo {
  construct new() {}
  toString { "Foo.toString" }
}

IO.print([1, Foo.new(), 2].join(", ")) // expect: 1, Foo.toString, 2

// TODO: Handle lists that contain themselves.
