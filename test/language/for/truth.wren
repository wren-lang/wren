class Iter {
  construct new(value) { _value = value }
  iterate(iterator) { _value }
  iteratorValue(iterator) { "value" }
}

// False and null are false.
for (n in Iter.new(false)) {
  System.print("bad")
  break
}

for (n in Iter.new(null)) {
  System.print("bad")
  break
}

// Everything else is true.
for (n in Iter.new(true)) {
  System.print("true") // expect: true
  break
}

for (n in Iter.new(0)) {
  System.print(0) // expect: 0
  break
}

for (n in Iter.new("")) {
  System.print("string") // expect: string
  break
}
