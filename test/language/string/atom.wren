// A string that starts with $ and ends when a non valid identifier character is found
// contains the same text.
// Useful for Map keys.

System.print($hello) // expect: hello

System.print({ $hello: "Wren" }) // expect: {hello: Wren}