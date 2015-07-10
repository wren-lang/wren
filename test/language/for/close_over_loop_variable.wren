var list = []

for (i in [1, 2, 3]) {
  list.add(Fn.new { IO.print(i) })
}

for (f in list) f.call()
// expect: 1
// expect: 2
// expect: 3
