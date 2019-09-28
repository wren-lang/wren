class Compound {
  value { _value }
  construct new() {
    _value = 0
  }

  plus() {
    _value += 5
  }
  minus() {
    _value -= 1
  }
  star() {
    _value *= 4
  }
  slash() {
    _value /= 2
  }
}

var a = Compound.new()
System.print(a.value) // expect: 0
a.plus()
System.print(a.value) // expect: 5
a.minus()
System.print(a.value) // expect: 4
a.star()
System.print(a.value) // expect: 16
a.slash()
System.print(a.value) // expect: 8