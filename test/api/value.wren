class Api {
  foreign static value=(value)
  foreign static value
}

Api.value = ["list", "of", "strings"]

// Do some stuff to trigger a GC (at least when GC stress testing enabled).
var s = "string"
for (i in 1...10) {
  s = s + " more"
}

System.print(Api.value) // expect: [list, of, strings]
