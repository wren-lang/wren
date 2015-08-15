// Class with a default constructor.
foreign class Counter {
  foreign increment(amount)
  foreign value
}

var counter = Counter.new()
IO.print(counter.value) // expect: 0
counter.increment(3.1)
IO.print(counter.value) // expect: 3.1
counter.increment(1.2)
IO.print(counter.value) // expect: 4.3

// Foreign classes can inherit a class as long as it has no fields.
class PointBase {
  inherited() {
    IO.print("inherited method")
  }
}

// Class with non-default constructor.
foreign class Point is PointBase {
  construct new() {
    IO.print("default")
  }

  construct new(x, y, z) {
    IO.print(x, ", ", y, ", ", z)
  }

  foreign translate(x, y, z)
  foreign toString
}

var p = Point.new(1, 2, 3) // expect: 1, 2, 3
IO.print(p) // expect: (1, 2, 3)
p.translate(3, 4, 5)
IO.print(p) // expect: (4, 6, 8)

p = Point.new() // expect: default
IO.print(p) // expect: (0, 0, 0)

p.inherited() // expect: inherited method

var error = Fiber.new {
  class Subclass is Point {}
}.try()
IO.print(error) // expect: Class 'Subclass' cannot inherit from foreign class 'Point'.
