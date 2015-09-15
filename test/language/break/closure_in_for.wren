var f
for (i in [1, 2, 3]) {
  var j = 4
  f = Fn.new { System.print(i + j) }
  break
}

f.call()
// expect: 5
