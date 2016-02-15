// A million fiber microbenchmark based on: https://github.com/atemerev/skynet.
class Skynet {
  static makeFiber(num, size, div) {
    return Fiber.new {
      if (size == 1) {
        Fiber.yield(num)
      } else {
        var fibers = []
        for (i in 0...div) {
          var subNum = num + i * (size / div)
          fibers.add(makeFiber(subNum, size / div, div))
        }

        var sum = 0
        for (task in fibers) {
          sum = sum + task.call()
        }
        Fiber.yield(sum)
      }
    }
  }
}

var start = System.clock
var result = Skynet.makeFiber(0, 1000000, 10).call()
var end = System.clock
System.print("Result: %(result) in %(end - start) s")
