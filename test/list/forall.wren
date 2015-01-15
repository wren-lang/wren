var a = [1, 2, 3]
var b = a.all {|x| x > 1 }
IO.print(b) // expect: false

var d = a.all {|x| x > 0 }
IO.print(d) // expect: true

var e = [].all {|x| false }
IO.print(e) // expect: true
