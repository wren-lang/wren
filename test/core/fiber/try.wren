var fiber = Fiber.new {
  System.print("before")
  true.unknownMethod
  System.print("after")
}

System.print(fiber.try())
// expect: before
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try

var fiber2 = Fiber.new {
  var fiberInner = Fiber.new {
    System.print("before")
    true.unknownMethod
    System.print("after")
  }
  fiberInner.call()
}

System.print(fiber2.try())
// expect: before
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try

var fiber3 = Fiber.new {
  var fiberInner = Fiber.new {
    var fiberInnerInner = Fiber.new {
      System.print("before")
      true.unknownMethod
      System.print("after")
    }
    fiberInnerInner.call()
  }
  fiberInner.call()
}

System.print(fiber3.try())
// expect: before
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try
