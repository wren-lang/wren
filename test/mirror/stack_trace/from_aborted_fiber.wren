
import "mirror" for StackTrace

var fn = Fn.new { Fiber.abort(42) }
var fiber = Fiber.new { fn.call() }

fiber.try()

System.print(StackTrace.new(fiber).count) // expect: 2
