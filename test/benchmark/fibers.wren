// Creates 100000 fibers. Each one calls the next in a chain until the last.
var fibers = []
var sum = 0

var start = System.clock

for (i in 0...100000) {
  fibers.add(Fiber.new {
    sum = sum + i
    if (i < 99999) fibers[i + 1].call()
  })
}

fibers[0].call()
System.print(sum)
System.print("elapsed: %(System.clock - start)")
