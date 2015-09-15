var f
while (true) {
  var i = "i"
  f = Fn.new { System.print(i) }
  break
}

f.call()
// expect: i
