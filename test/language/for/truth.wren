class Iter {
  construct new(value) { _value = value }
  iterate(iterator) { _value }
  iteratorValue(iterator) { "value" }
}

// False and null are false.
for (n in Iter.new(false)) {
  IO.print("bad")
  break
}

for (n in Iter.new(null)) {
  IO.print("bad")
  break
}

// Everything else is true.
for (n in Iter.new(true)) {
  IO.print("true") // expect: true
  break
}

for (n in Iter.new(0)) {
  IO.print(0) // expect: 0
  break
}

for (n in Iter.new("")) {
  IO.print("string") // expect: string
  break
}
