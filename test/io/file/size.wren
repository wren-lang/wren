import "io" for File
import "scheduler" for Scheduler

System.print(File.size("test/io/file/size.wren")) // expect: 401

// Runs asynchronously.
Scheduler.add {
  System.print("async")
}

System.print(File.size("test/io/file/size.wren"))
// expect: async
// expect: 401

var error = Fiber.new {
  System.print(File.size("nonexistent"))
}.try()
System.print(error) // expect: no such file or directory
