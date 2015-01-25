var a = {"one": 1, "two": 2, "three": 3, "four": 4}.keys

// The precise numeric values aren't defined since they are indexes into the
// entry table and the hashing process isn't specified. So we just validate
// what we can assume about them.

IO.print(a.iterate(null) is Num) // expect: true
IO.print(a.iterate(null) >= 0) // expect: true

IO.print(a.iterate(0) is Num) // expect: true
IO.print(a.iterate(0) > 0) // expect: true
IO.print(a.iterate(1) is Num) // expect: true
IO.print(a.iterate(1) > 0) // expect: true
IO.print(a.iterate(2) is Num) // expect: true
IO.print(a.iterate(2) > 0) // expect: true
IO.print(a.iterate(3) is Num) // expect: true
IO.print(a.iterate(3) > 0) // expect: true

var previous = -1
var iterator = a.iterate(null)
while (iterator) {
  IO.print(iterator > previous)
  IO.print(iterator is Num)
  previous = iterator
  iterator = a.iterate(iterator)
}
// First entry:
// expect: true
// expect: true
// Second entry:
// expect: true
// expect: true
// Third entry:
// expect: true
// expect: true
// Fourth entry:
// expect: true
// expect: true

// Out of bounds.
IO.print(a.iterate(16)) // expect: false
IO.print(a.iterate(-1)) // expect: false

// Nothing to iterate in an empty map.
IO.print({}.keys.iterate(null)) // expect: false
