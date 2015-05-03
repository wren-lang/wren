// Handle empty map.
IO.print({}.toString)             // expect: {}

// Does not quote strings.
IO.print({"1": "2"}.toString) // expect: {1: 2}

// Nested maps.
IO.print({1: {2: {}}}) // expect: {1: {2: {}}}

// Calls toString on elements.
class Foo {
  toString { "Foo.toString" }
}

IO.print({1: new Foo}) // expect: {1: Foo.toString}

// Since iteration order is unspecified, we don't know what order the results
// will be.
var s = {1: 2, 3: 4, 5: 6}.toString
IO.print(s == "{1: 2, 3: 4, 5: 6}" ||
         s == "{1: 2, 5: 6, 3: 4}" ||
         s == "{3: 4, 1: 2, 5: 6}" ||
         s == "{3: 4, 5: 6, 1: 2}" ||
         s == "{5: 6, 1: 2, 3: 4}" ||
         s == "{5: 6, 3: 4, 1: 2}") // expect: true

// Map that directly contains itself.
var map = {}
map["key"] = map
IO.print(map) // expect: {key: ...}

// Map that indirectly contains itself.
map = {}
map["a"] = {"b": {"c": map}}

IO.print(map) // expect: {a: {b: {c: ...}}}

// Map containing an object that calls toString on a recursive map.
class Box {
  new(field) { _field = field }
  toString { "box " + _field.toString }
}

map = {}
map["box"] = new Box(map)
IO.print(map) // expect: {box: box ...}

// Map containing a list containing the map.
map = {}
map["list"] = [1, map, 2]
IO.print(map) // expect: {list: [1, ..., 2]}
