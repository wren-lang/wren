var a = ["one", "two", "three", "four"]
IO.write(a.iterate(null)) // expect: 0
IO.write(a.iterate(0)) // expect: 1
IO.write(a.iterate(1)) // expect: 2
IO.write(a.iterate(2)) // expect: 3
IO.write(a.iterate(3)) // expect: false
IO.write(a.iterate(-1)) // expect: false
