var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

IO.print(map.containsKey("one")) // expect: true
IO.print(map.containsKey("two")) // expect: true
IO.print(map.containsKey("three")) // expect: true
IO.print(map.containsKey("four")) // expect: false
IO.print(map.containsKey("five")) // expect: false
