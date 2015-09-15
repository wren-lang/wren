for (i in [1, 2, 3, 4, 5]) {
  System.print(i)
  if (i > 2) break
  System.print(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
