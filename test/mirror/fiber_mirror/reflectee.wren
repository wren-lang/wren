
import "mirror" for Mirror, FiberMirror

var fiber = Fiber.new {
  Fiber.yield()
}

var mirror = Mirror.reflect(fiber)

System.print(mirror is FiberMirror)     // expect: true
System.print(mirror.reflectee == fiber) // expect: true
