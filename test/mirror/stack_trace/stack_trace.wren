
import "mirror" for StackTrace

var fn = Fn.new { Fiber.yield() }
var fiber = Fiber.new { fn.call() }

var stackTrace = StackTrace.new(fiber)
System.print(stackTrace.count) // expect: 1

fiber.call()

// Check immutability
System.print(stackTrace.count) // expect: 1

System.print(StackTrace.new(fiber).count) // expect: 2

fiber.call()

// Check for termination
System.print(StackTrace.new(fiber).count) // expect: 0
