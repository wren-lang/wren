var a = "before"
io.write(a) // expect: before

a = "after"
io.write(a) // expect: after

io.write(a = "arg") // expect: arg
io.write(a) // expect: arg
