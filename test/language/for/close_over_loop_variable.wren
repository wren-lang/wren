var list = []

for (i in [1, 2, 3]) {
  list.add(fn () { System.print(i) })
}

for (f in list) f()
// expect: 1
// expect: 2
// expect: 3
