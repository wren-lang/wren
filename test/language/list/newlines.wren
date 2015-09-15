// Allow after '[' and ',', and before ']'.
var list = [

"a",
"b"

]

System.print(list[0]) // expect: a
System.print(list[1]) // expect: b

// Newline after trailing comma.
list = ["c",

]

System.print(list[0]) // expect: c

// Newline in empty list.
list = [

]

System.print(list.count) // expect: 0
