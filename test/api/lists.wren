class Lists {
  foreign static newList()
  foreign static insert()
}

var list = Lists.newList()
System.print(list is List) // expect: true
System.print(list.count) // expect: 0

System.print(Lists.insert()) // expect: [4, 5, 6, 1, 2, 3, 9, 8, 7]
