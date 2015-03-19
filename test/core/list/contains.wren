var list = [1, 2, 3, 4, "foo"]

IO.print(list.contains(2))      // expect: true
IO.print(list.contains(5))      // expect: false
IO.print(list.contains("foo"))  // expect: true
IO.print(list.contains("bar"))  // expect: false
