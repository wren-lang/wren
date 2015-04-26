var m = {}
var l = []
var r = 1..2
var x = 42

var f = new Fn {
}
class Test {
}
var t = new Test
var n

IO.print(typeof(m) == "Map") // expect: true
IO.print(typeof(l) == "List") // expect: true
IO.print(typeof(r) == "Range") // expect: true
IO.print(typeof(x) == "Num") // expect: true
IO.print(typeof(f) == "Fn") // expect: true
IO.print(typeof(t) == "Test") // expect: true
IO.print(typeof(n) == "Null") // expect: true
