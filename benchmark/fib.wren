var fib = fn(n) {
  if (n < 2) {
    return n
  }

  return fib.call(n - 1) + fib.call(n - 2)
}

var start = OS.clock
for (i in 1..5) {
  IO.print(fib.call(28))
}
IO.print("elapsed: " + (OS.clock - start).toString)
