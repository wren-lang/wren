import "example" for Example
var cc = Fiber.current

var x = Fiber.new {
  System.print("hello 1")
  Example.exampleAdd(1.0, 2.0)
  Fiber.yield("foo")
  System.print("hello 2")
  Example.exampleStr("abc", "def")
  Fiber.yield("bar")
}

var val = x.call()
System.print(val)

val = x.call()
System.print(val)



