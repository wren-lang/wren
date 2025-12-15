
import "mirror" for StackTrace

System.print(StackTrace.new(Fiber.current).count) // expect: 1

var fn = Fn.new { StackTrace.new(Fiber.current) }
var fiber = Fiber.new { fn.call() }

System.print(fiber.call().count) // expect: 2
