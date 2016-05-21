class Handle {
  foreign static value=(value)
  foreign static value
}

Handle.value = ["list", "of", "strings"]

// Make sure the handle lives through a GC.
System.gc()

System.print(Handle.value) // expect: [list, of, strings]
