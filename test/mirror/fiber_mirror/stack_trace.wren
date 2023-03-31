
import "mirror" for Mirror

var fn = Fn.new { Fiber.yield() }
var fiber = Fiber.new { fn.call() }

var stackTrace = Mirror.reflect(fiber).stackTrace()
System.print(stackTrace.count) // expect: 1

fiber.call()

// Check immutability
System.print(stackTrace.count) // expect: 1

System.print(Mirror.reflect(fiber).stackTrace().count) // expect: 2

fiber.call()

// Check for termination
System.print(Mirror.reflect(fiber).stackTrace().count) // expect: 0
