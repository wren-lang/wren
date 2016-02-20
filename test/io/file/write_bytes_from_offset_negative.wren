import "io" for File

System.print(Fiber.new {
  File.create("file.temp") {|file|
    file.writeBytes("", -1)
  }
}.try()) // expect: Offset cannot be negative.

File.delete("file.temp")
