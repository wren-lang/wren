var fiber = Fiber.new {
  System.print("fiber 1")

  import "./yield_from_import_module"

  System.print("fiber 2")
}

fiber.call()              // expect: fiber 1
                          // expect: module 1
System.print("main 1")    // expect: main 1
fiber.call()              // expect: module 2
                          // expect: fiber 2
System.print("main 2")    // expect: main 2
