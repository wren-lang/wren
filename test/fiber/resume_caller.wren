var b = new Fiber {
  IO.print("fiber b")
}

var a = new Fiber {
  IO.print("begin fiber a")
  b.run
  IO.print("end fiber a")
}

IO.print("begin main")
a.run
IO.print("end main")

// expect: begin main
// expect: begin fiber a
// expect: fiber b
// expect: end fiber a
// expect: end main
