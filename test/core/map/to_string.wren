// Handle empty map.
System.print({}.toString)             // expect: {}

// Does not quote strings.
System.print({"1": "2"}.toString) // expect: {1: 2}

// Nested maps.
System.print({1: {2: {}}}) // expect: {1: {2: {}}}

// Calls toString on elements.
class Foo {
  construct new() {}
  toString { "Foo.toString" }
}

System.print({1: Foo.new()}) // expect: {1: Foo.toString}

// Since iteration order is unspecified, we don't know what order the results
// will be.
var s = {1: 2, 3: 4, 5: 6}.toString
System.print(s == "{1: 2, 3: 4, 5: 6}" ||
             s == "{1: 2, 5: 6, 3: 4}" ||
             s == "{3: 4, 1: 2, 5: 6}" ||
             s == "{3: 4, 5: 6, 1: 2}" ||
             s == "{5: 6, 1: 2, 3: 4}" ||
             s == "{5: 6, 3: 4, 1: 2}") // expect: true

// TODO: Handle maps that contain themselves.
