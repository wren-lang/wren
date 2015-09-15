var i = 0
while (true) {
  i = i + 1
  System.print(i)
  if (i > 2) break
  System.print(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
