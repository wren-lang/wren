var b = Fiber.create(fn {
  IO.print("fiber b")
})

var a = Fiber.create(fn {
  IO.print("begin fiber a")
  b.run
  IO.print("end fiber a")
})

IO.print("begin main")
a.run
IO.print("end main")

// expect: begin main
// expect: begin fiber a
// expect: fiber b
// expect: end fiber a
// expect: end main
