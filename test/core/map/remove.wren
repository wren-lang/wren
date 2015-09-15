var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

System.print(map.count) // expect: 3
System.print(map.remove("two")) // expect: 2
System.print(map.count) // expect: 2
System.print(map.remove("three")) // expect: 3
System.print(map.count) // expect: 1

// Remove an already removed entry.
System.print(map.remove("two")) // expect: null
System.print(map.count) // expect: 1

System.print(map.remove("one")) // expect: 1
System.print(map.count) // expect: 0
