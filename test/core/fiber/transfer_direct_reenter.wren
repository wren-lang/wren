var f = Fiber.new {
  System.print(1)            // expect: 1
  System.print(f.transfer()) // expect: null
  System.print(2)            // expect: 2
}

f()
// f remembers its original caller so transfers back to main.
System.print(3) // expect: 3
