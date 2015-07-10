var f
while (true) {
  var i = "i"
  f = Fn.new { IO.print(i) }
  break
}

f.call()
// expect: i
