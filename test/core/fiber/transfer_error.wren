var a = Fiber.new {
  System.print("transferred to a")
  b.transferError("error!")
}

var b = Fiber.new {
  System.print("started b")
  a.transfer()
  System.print("should not get here")
}

b.try()
// expect: started b
// expect: transferred to a
System.print(b.error) // expect: error!
