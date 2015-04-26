class Test {
  new {
    _intField = 42
    __staticField = {}
  }

  check {
    return typeof(_intField) == Num && typeof(__staticField) == Map
  }
}

var t = new Test
IO.print(t.check) // expect: true
