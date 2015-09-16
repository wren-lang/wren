// Handle empty list.
System.print([].join(",") == "") // expect: true

// Handle a simple list with an empty delimeter.
System.print([1, 2, 3].join("")) // expect: 123

// Handle a simple list with no separator.
System.print([1, 2, 3].join()) // expect: 123

// Does not quote strings.
System.print([1, "2", true].join(",")) // expect: 1,2,true

// Nested lists.
System.print([1, [2, [3], 4], 5].join(",")) // expect: 1,[2, [3], 4],5

// Calls toString on elements.
class Foo {
  construct new() {}
  toString { "Foo.toString" }
}

System.print([1, Foo.new(), 2].join(", ")) // expect: 1, Foo.toString, 2

// TODO: Handle lists that contain themselves.
