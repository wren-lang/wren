class Lists {
  foreign static newList()
  foreign static insert()
  foreign static set()
  foreign static get(list, index)
}

var list = Lists.newList()
System.print(list is List) // expect: true
System.print(list.count) // expect: 0

System.print(Lists.insert()) // expect: [4, 5, 6, 1, 2, 3, 9, 8, 7]

System.print(Lists.set()) // expect: [1, 2, 33, 44]
System.print(Lists.get([1,2,3,4], -2)) // expect: 3
System.print(Lists.get([1,2,3,4], 1)) // expect: 2
