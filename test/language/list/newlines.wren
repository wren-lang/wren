// Allow after '[' and ',', and before ']'.
var list = [

"a",
"b"

]

IO.print(list[0]) // expect: a
IO.print(list[1]) // expect: b

// Newline after trailing comma.
list = ["c",

]

IO.print(list[0]) // expect: c

// Newline in empty list.
list = [

]

IO.print(list.count) // expect: 0
