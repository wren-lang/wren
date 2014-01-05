var i = 0
while (true) {
  i = i + 1
  IO.print(i)
  if (i > 2) break
  IO.print(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
