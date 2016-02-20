import "io" for File

System.print(Fiber.new {
  File.create("file.temp") {|file|
    file.writeBytes(123)
  }
}.try()) // expect: Bytes must be a string.

File.delete("file.temp")
