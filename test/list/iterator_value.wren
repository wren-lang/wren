var a = ["one", "two", "three", "four"]
IO.write(a.iteratorValue(0)) // expect: one
IO.write(a.iteratorValue(1)) // expect: two
IO.write(a.iteratorValue(2)) // expect: three
IO.write(a.iteratorValue(3)) // expect: four
