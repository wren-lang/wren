import "io" for File

System.print(Fiber.new {
  File.create("file.temp") {|file|
    file.writeBytes("", 1.2)
  }
}.try()) // expect: Offset must be an integer.

File.delete("file.temp")
