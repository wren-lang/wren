foreign class RawValueHolder {
  construct new() {}
  construct new(val) {
    value = val
  }

  foreign value
  foreign value=(rhs)
  foreign static count
}

System.print(RawValueHolder.count) // expect: 0
var h1 = RawValueHolder.new()
System.print(RawValueHolder.count) // expect: 1
h1.value = 123
System.print(h1.value) // expect: 123
System.gc()
System.print(h1.value) // expect: 123
h1.value = "Hello, world!"
System.print(h1.value) // expect: Hello, world!
System.gc()
System.print(h1.value) // expect: Hello, world!
System.print(RawValueHolder.count) // expect: 1
h1.value = RawValueHolder.new("Boop")
System.print(RawValueHolder.count) // expect: 2
System.print(h1.value.value) // expect: Boop
System.gc()
System.print(RawValueHolder.count) // expect: 2
System.print(h1.value.value) // expect: Boop
h1.value = null
System.gc()
System.print(RawValueHolder.count) // expect: 1

h1.value = RawValueHolder.new()
h1.value.value = RawValueHolder.new()
System.print(RawValueHolder.count) // expect: 3
h1.value = null
System.gc()
System.print(RawValueHolder.count) // expect: 1
h1 = null
System.gc()
System.print(RawValueHolder.count) // expect: 0
