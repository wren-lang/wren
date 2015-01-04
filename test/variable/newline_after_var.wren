var
foo = 123

IO.print(foo) // expect: 123

var bar, baz
bar = 123

IO.print(bar) // expect: 123
IO.print(baz) // expect: null

var a = "ok",
    b = 123

IO.print(a) // expect: ok
IO.print(b) // expect: 123

var
    // This is a comment about the first variable.
    c = "ok",

    // This is a comment about the next set of variables.
    d = 123,
    e = 234

IO.print(c) // expect: ok
IO.print(d) // expect: 123
IO.print(e) // expect: 234