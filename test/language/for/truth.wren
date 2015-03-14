class Iter {
  new(value) { _value = value }
  iterate(iterator) { _value }
  iteratorValue(iterator) { "value" }
}

// False and null are false.
for (n in new Iter(false)) {
  IO.print("bad")
  break
}

for (n in new Iter(null)) {
  IO.print("bad")
  break
}

// Everything else is true.
for (n in new Iter(true)) {
  IO.print("true") // expect: true
  break
}

for (n in new Iter(0)) {
  IO.print(0) // expect: 0
  break
}

for (n in new Iter("")) {
  IO.print("string") // expect: string
  break
}
