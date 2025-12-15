
import "mirror" for FiberMirror

var mirror1 = FiberMirror.current

System.print(mirror1.reflectee == Fiber.current) // expect: true

var mirror2 = FiberMirror.current

System.print(mirror1 == mirror2) // expect: true
System.print(mirror1 != mirror2) // expect: false
