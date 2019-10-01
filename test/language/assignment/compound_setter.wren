class Compound {
  value { _value }
  value=(v) { _value = v }
  other { _other }
  other=(v) { _other = v }
  construct new() {
    _value = 0
  }
}

var a = Compound.new()
a.value = 0
System.print(a.value) // expect: 0
a.value += 5
System.print(a.value) // expect: 5
a.value -= 1
System.print(a.value) // expect: 4
a.value *= 4
System.print(a.value) // expect: 16
a.value /= 2
System.print(a.value) // expect: 8
a.value <<= 1
System.print(a.value) // expect: 16
a.value >>= 2
System.print(a.value) // expect: 4

a.other = Compound.new()
a.other.value = 4
System.print(a.other.value) // expect: 4
a.other.value += 10
System.print(a.other.value) // expect: 14
a.other.value -= 2
System.print(a.other.value) // expect: 12
a.other.value *= 2
System.print(a.other.value) // expect: 24
a.other.value /= 6
System.print(a.other.value) // expect: 4
a.other.value <<= 4
System.print(a.other.value) // expect: 64
a.other.value >>= 3
System.print(a.other.value) // expect: 8

a.other.other = Compound.new()
a.other.other.value = 2
System.print(a.other.other.value) // expect: 2
a.other.other.value += 8
System.print(a.other.other.value) // expect: 10
a.other.other.value *= 10
System.print(a.other.other.value) // expect: 100
a.other.other.value -= 1
System.print(a.other.other.value) // expect: 99
a.other.other.value /= 3
System.print(a.other.other.value) // expect: 33
a.other.other.value <<= 1
System.print(a.other.other.value) // expect: 66
a.other.other.value >>= 2
System.print(a.other.other.value) // expect: 16