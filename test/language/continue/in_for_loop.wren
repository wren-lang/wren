for (i in [1, 2, 3, 4, 5]) {
  System.print(i)
  if (i > 2) continue
  System.print(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
// expect: 4
// expect: 5
