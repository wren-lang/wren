
class Compound {
  value { _value }
  value=(v) { _value = v }
  other { _other }
  other=(v) { _other = v }

  [index, other, last] { _value }
  [index, other, last]=(v) { _value = v }

  construct new() {
    _value = 0
  }
}

var a = Compound.new()
a[0,1,2] = 0
System.print(a[0,1,2]) // expect: 0
a[0,1,2] += 5
System.print(a[0,1,2]) // expect: 5
a[0,1,2] -= 1
System.print(a[0,1,2]) // expect: 4
a[0,1,2] *= 4
System.print(a[0,1,2]) // expect: 16
a[0,1,2] /= 2
System.print(a[0,1,2]) // expect: 8

a.other = Compound.new()
a.other[0,1,2] = 4
System.print(a.other[0,1,2]) // expect: 4
a.other[0,1,2] += 10
System.print(a.other[0,1,2]) // expect: 14
a.other[0,1,2] -= 2
System.print(a.other[0,1,2]) // expect: 12
a.other[0,1,2] *= 2
System.print(a.other[0,1,2]) // expect: 24
a.other[0,1,2] /= 6
System.print(a.other[0,1,2]) // expect: 4

a.other.other = Compound.new()
a.other.other[1,2,3] = 2
System.print(a.other.other[1,2,3]) // expect: 2
a.other.other[1,2,3] += 8
System.print(a.other.other[1,2,3]) // expect: 10
a.other.other[1,2,3] *= 10
System.print(a.other.other[1,2,3]) // expect: 100
a.other.other[1,2,3] -= 1
System.print(a.other.other[1,2,3]) // expect: 99
a.other.other[1,2,3] /= 3
System.print(a.other.other[1,2,3]) // expect: 33


