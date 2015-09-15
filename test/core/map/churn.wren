// This is a regression test for a bug where inserting in a map would not
// correctly reuse tombstone entries, eventually deadlocking on insert.
var map = {}

for (i in 0...100) {
  map[i] = i

  if (i >= 10) map.remove(i - 10)
}

System.print(map.count) // expect: 10
