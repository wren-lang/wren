
import "mirror" for Mirror

var fn = Fn.new { Fiber.abort(42) }
var fiber = Fiber.new { fn.call() }

fiber.try()

System.print(Mirror.reflect(fiber).stackTrace().count) // expect: 2
