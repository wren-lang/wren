var list = []

for (i in [1, 2, 3]) {
  var j = i + 1
  list.add(fn () { System.print(j) })
}

for (f in list) f()
// expect: 2
// expect: 3
// expect: 4
