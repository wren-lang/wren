class Fib {
  static get(n) {
    if (n < 2) return n
    return get(n - 1) + get(n - 2)
  }
}

var start = System.clock
for (i in 1..5) {
  System.print(Fib.get(28))
}
System.print("elapsed: %(System.clock - start)")
