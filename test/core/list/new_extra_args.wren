var list = List.new(5)

System.print(list.count) // expect: 5
System.print(list) // expect: [null, null, null, null, null]

var list2 = List.new(5, 2)
System.print(list2.count) // expect: 5
System.print(list2) // expect: [2, 2, 2, 2, 2]
