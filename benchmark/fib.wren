var fib = fn(n) {
  if (n < 2) {
    return n
  }

  return fib.call(n - 1) + fib.call(n - 2)
}

var start = OS.clock
for (i in 1..5) {
  IO.write(fib.call(28))
}
IO.write("elapsed: " + (OS.clock - start).toString)
