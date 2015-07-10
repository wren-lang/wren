var b = Fiber.new {
  IO.print("fiber b")
}

var a = Fiber.new {
  IO.print("begin fiber a")
  b.call()
  IO.print("end fiber a")
}

IO.print("begin main")
a.call()
IO.print("end main")

// expect: begin main
// expect: begin fiber a
// expect: fiber b
// expect: end fiber a
// expect: end main
