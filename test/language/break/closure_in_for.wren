var f
for (i in [1, 2, 3]) {
  var j = 4
  f = fn () { System.print(i + j) }
  break
}

f()
// expect: 5
