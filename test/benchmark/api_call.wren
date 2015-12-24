class Benchmark {
  foreign static call(iterations)
}

var result = Benchmark.call(1000000)
// Returns false if it didn't calculate the right value. Otherwise returns the
// elapsed time.
System.print(result is Num)
System.print("elapsed: %(result)")
