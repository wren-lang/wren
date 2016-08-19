var a = {1: "one"}

// The actual iterator values are implementation specific, so ask the map.
var iterator = a.iterate(null)
var value = a.iteratorValue(iterator)

System.print(value is MapEntry) // expect: true
System.print(value.key)         // expect: 1
System.print(value.value)       // expect: one

// The entry does not track the underlying map.
a[1] = "updated"
System.print(value.value)       // expect: one
