var i = 0
while (true) {
  i = i + 1
  System.print(i)
  if (i <= 2) continue
  System.print(i)
  break
}
// expect: 1
// expect: 2
// expect: 3
// expect: 3
