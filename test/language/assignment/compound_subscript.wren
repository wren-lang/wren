
class Compound {
  value { _value }
  value=(v) { _value = v }
  other { _other }
  other=(v) { _other = v }

  [index] { _value }
  [index]=(v) { _value = v }

  [index, other, last] { _value }
  [index, other, last]=(v) { _value = v }

  construct new() {
    _value = 0
  }
}

var a = Compound.new()
a[0] = 0
System.print(a[0]) // expect: 0
a[0] += 5
System.print(a[0]) // expect: 5
a[0] -= 1
System.print(a[0]) // expect: 4
a[0] *= 4
System.print(a[0]) // expect: 16
a[0] /= 2
System.print(a[0]) // expect: 8
a[0] <<= 2
System.print(a[0]) // expect: 32
a[0] >>= 4
System.print(a[0]) // expect: 2

a.other = Compound.new()
a.other[0] = 4
System.print(a.other[0]) // expect: 4
a.other[0] += 10
System.print(a.other[0]) // expect: 14
a.other[0] -= 2
System.print(a.other[0]) // expect: 12
a.other[0] *= 2
System.print(a.other[0]) // expect: 24
a.other[0] /= 6
System.print(a.other[0]) // expect: 4
a.other[0] <<= 5
System.print(a.other[0]) // expect: 128
a.other[0] >>= 3
System.print(a.other[0]) // expect: 16

a.other.other = Compound.new()
a.other.other[1] = 2
System.print(a.other.other[1]) // expect: 2
a.other.other[1] += 8
System.print(a.other.other[1]) // expect: 10
a.other.other[1] *= 10
System.print(a.other.other[1]) // expect: 100
a.other.other[1] -= 1
System.print(a.other.other[1]) // expect: 99
a.other.other[1] /= 3
System.print(a.other.other[1]) // expect: 33
a.other.other[1] <<= 1
System.print(a.other.other[1]) // expect: 66
a.other.other[1] >>= 3
System.print(a.other.other[1]) // expect: 8


