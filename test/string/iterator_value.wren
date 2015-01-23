var s = "abçd"
IO.print(s.iteratorValue(0)) // expect: a
IO.print(s.iteratorValue(1)) // expect: b
IO.print(s.iteratorValue(2)) // expect: ç
// Iterator value in middle of UTF sequence is an empty string.
IO.print(s.iteratorValue(3) == "") // expect: true
IO.print(s.iteratorValue(4)) // expect: d
