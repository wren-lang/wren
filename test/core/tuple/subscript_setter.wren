
var tuple = Tuple.new(0, 1, 2, 3)
System.print(tuple[0] = 3) // expect: 3
System.print(tuple[1] = 2) // expect: 2
System.print(tuple[2] = 1) // expect: 1
System.print(tuple[3] = 0) // expect: 0

System.print(tuple[0]) // expect: 3
System.print(tuple[1]) // expect: 2
System.print(tuple[2]) // expect: 1
System.print(tuple[3]) // expect: 0

System.print(tuple[-1] = 3) // expect: 3
System.print(tuple[-2] = 2) // expect: 2
System.print(tuple[-3] = 1) // expect: 1
System.print(tuple[-4] = 0) // expect: 0

System.print(tuple[0]) // expect: 0
System.print(tuple[1]) // expect: 1
System.print(tuple[2]) // expect: 2
System.print(tuple[3]) // expect: 3
