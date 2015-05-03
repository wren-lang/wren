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

// Lists that directly contain themselves.
var list = []
list.add(list)
IO.print(list) // expect: [...]

list = [1, 2]
list[0] = list
IO.print(list) // expect: [..., 2]

list = [1, 2]
list[1] = list
IO.print(list) // expect: [1, ...]

// Lists that indirectly contain themselves.
list = [null, [2, [3, null, 4], null, 5], 6]
list[0] = list
list[1][1][1] = list
list[1][2] = list
IO.print(list) // expect: [..., [2, [3, ..., 4], ..., 5], 6]

// List containing an object that calls toString on a recursive list.
class Box {
  new(field) { _field = field }
  toString { "box " + _field.toString }
}

list = [1, 2]
list.add(new Box(list))
IO.print(list) // expect: [1, 2, box ...]

// List containing a map containing the list.
list = [1, null, 2]
list[1] = {"list": list}
IO.print(list) // expect: [1, {list: ...}, 2]
