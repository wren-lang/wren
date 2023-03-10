System.print(Tuple.new().count) // expect: 0
System.print(Tuple.new(0).count) // expect: 1
System.print(Tuple.new(0, 1).count) // expect: 2
System.print(Tuple.new(0, 1, 2).count) // expect: 3
System.print(Tuple.new(0, 1, 2, 3).count) // expect: 4
