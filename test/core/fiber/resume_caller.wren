var b = Fiber.new {
  System.print("fiber b")
}

var a = Fiber.new {
  System.print("begin fiber a")
  b.call()
  System.print("end fiber a")
}

System.print("begin main")
a.call()
System.print("end main")

// expect: begin main
// expect: begin fiber a
// expect: fiber b
// expect: end fiber a
// expect: end main
