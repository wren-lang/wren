// Regression test for #373.
var map = {}
map[2] = "two"
map[0] = "zero"
map.remove(2)
map[0] = "zero again"
map.remove(0)

System.print(map.containsKey(0)) // expect: false
