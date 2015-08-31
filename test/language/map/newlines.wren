// Allow after '{', ':', and ',', and before ']'.
var map = {

"a":

"a value",

"b": "b value"

}

IO.print(map["a"]) // expect: a value
IO.print(map["b"]) // expect: b value

// Newline after trailing comma.
map = {"c": "c value",

}

IO.print(map["c"]) // expect: c value

// Newline in empty map.
map = {

}

IO.print(map.count) // expect: 0
