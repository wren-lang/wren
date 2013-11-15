var fib = fn(n) {
  if (n < 2) {
    n
  } else {
    fib.call(n - 1) + fib.call(n - 2)
  }
}

io.write(fib.call(35))
