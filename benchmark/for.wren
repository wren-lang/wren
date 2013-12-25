var list = []

{
  var i = 0
  while (i < 2000000) {
    list.add(i)
    i = i + 1
  }
}

var start = OS.clock
var sum = 0
for (i in list) {
  sum = sum + i
}
IO.write(sum)
IO.write("elapsed: " + (OS.clock - start).toString)
