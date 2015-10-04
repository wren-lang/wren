var A = Fiber.new {
  System.print("transferred to A")
  B.transferError("error!")
}

var B = Fiber.new {
  System.print("started B")
  A.transfer()
  System.print("should not get here")
}

B.try()
// expect: started B
// expect: transferred to A
System.print(B.error) // expect: error!
