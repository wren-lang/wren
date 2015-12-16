class Benchmark {
  foreign static arguments(a, b, c, d)
}

var start = System.clock
var result = 0
for (i in 1..1000000) {
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
  result = result + Benchmark.arguments(1, 2, 3, 4)
}
System.print(result)
System.print("elapsed: %(System.clock - start)")
