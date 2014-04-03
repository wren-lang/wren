var f = new Fn {
  IO.print("evaluate sequence")
  return [1, 2, 3]
}

for (i in f.call) IO.print(i)
// expect: evaluate sequence
// expect: 1
// expect: 2
// expect: 3
