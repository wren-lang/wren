class Api {
  foreign static gc()
  foreign static finalized
}

// Class with a default constructor.
foreign class Counter {
  construct new() {}
  foreign increment(amount)
  foreign value
}

var counter = Counter.new()
System.print(counter.value) // expect: 0
counter.increment(3.1)
System.print(counter.value) // expect: 3.1
counter.increment(1.2)
System.print(counter.value) // expect: 4.3

// Foreign classes can inherit a class as long as it has no fields.
class PointBase {
  inherited() {
    System.print("inherited method")
  }
}

// Class with non-default constructor.
foreign class Point is PointBase {
  construct new() {
    System.print("default")
  }

  construct new(x, y, z) {
    System.print(x.toString + ", " + y.toString + ", " + z.toString)
  }

  foreign translate(x, y, z)
  foreign toString
}

var p = Point.new(1, 2, 3) // expect: 1, 2, 3
System.print(p) // expect: (1, 2, 3)
p.translate(3, 4, 5)
System.print(p) // expect: (4, 6, 8)

p = Point.new() // expect: default
System.print(p) // expect: (0, 0, 0)

p.inherited() // expect: inherited method

var error = Fiber.new {
  class Subclass is Point {}
}.try()
System.print(error) // expect: Class 'Subclass' cannot inherit from foreign class 'Point'.

// Class with a finalizer.
foreign class Resource {
  construct new() {}
}

var resources = [
  Resource.new(),
  Resource.new(),
  Resource.new()
]

Api.gc()
System.print(Api.finalized) // expect: 0

resources.removeAt(-1)

Api.gc()
System.print(Api.finalized) // expect: 1

resources.clear()

Api.gc()
System.print(Api.finalized) // expect: 3
