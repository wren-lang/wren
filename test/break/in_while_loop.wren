var i = 0
while (true) {
  i = i + 1
  IO.write(i)
  if (i > 2) break
  IO.write(i)
}
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
