import "io" for Stdin

Stdin.readLine() // stdin: one line

var error = Fiber.new {
  Stdin.readLine()
}.try()

System.print(error) // expect: Stdin was closed.
