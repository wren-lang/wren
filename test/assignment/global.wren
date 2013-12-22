var a = "before"
IO.write(a) // expect: before

a = "after"
IO.write(a) // expect: after

IO.write(a = "arg") // expect: arg
IO.write(a) // expect: arg
