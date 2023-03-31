
import "mirror" for Mirror

System.print(Mirror.reflect(Fiber.current).stackTrace().count) // expect: 1

var fn = Fn.new { Mirror.reflect(Fiber.current).stackTrace() }
var fiber = Fiber.new { fn.call() }

System.print(fiber.call().count) // expect: 2
