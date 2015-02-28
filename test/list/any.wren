var a = [1, 2, 3]

var b = a.any {|x| x > 3 }
IO.print(b) // expect: false

var d = a.any {|x| x > 1 }
IO.print(d) // expect: true

var e = [].any {|x| true }
IO.print(e) // expect: false
