// Allow after '{', ':', and ',', and before ']'.
var map = {

"a":

"a value",

"b": "b value"

}

System.print(map["a"]) // expect: a value
System.print(map["b"]) // expect: b value

// Newline after trailing comma.
map = {"c": "c value",

}

System.print(map["c"]) // expect: c value

// Newline in empty map.
map = {

}

System.print(map.count) // expect: 0
