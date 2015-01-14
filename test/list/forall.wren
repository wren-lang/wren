var a = [1, 2, 3]
var b = a.forall {|x| x > 1 }
IO.print(b) // expect: false

var c = a.forall {|x| x.toString }
IO.print(c) // expect: false

var d = a.forall {|x| x > 0 }
IO.print(d) // expect: true
