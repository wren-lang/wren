var F = Fiber.new {
  IO.print(1) // expect: 1
  IO.print(F.transfer("value")) // expect: value
  IO.print(2) // expect: 2
}

F.call()
IO.print(3) // expect: 3
