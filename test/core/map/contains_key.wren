var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

System.print(map.containsKey("one")) // expect: true
System.print(map.containsKey("two")) // expect: true
System.print(map.containsKey("three")) // expect: true
System.print(map.containsKey("four")) // expect: false
System.print(map.containsKey("five")) // expect: false
