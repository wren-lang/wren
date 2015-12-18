class Value {
  foreign static def value=(value)
  foreign static def value
}

Value.value = ["list", "of", "strings"]

// Do some stuff to trigger a GC (at least when GC stress testing enabled).
var s = "string"
for (i in 1...10) {
  s = s + " more"
}

System.print(Value.value) // expect: [list, of, strings]
