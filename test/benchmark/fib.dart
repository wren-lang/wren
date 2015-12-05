fib(n) {
  if (n < 2) return n;
  return fib(n - 1) + fib(n - 2);
}

main() {
  Stopwatch watch = new Stopwatch();
  watch.start();
  for (var i = 0; i < 5; i++) {
    print(fib(28));
  }
  print("elapsed: ${watch.elapsedMilliseconds / 1000}");
}
