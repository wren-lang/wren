import "io" for File

var file = File.create("file.temp")
file.close()

System.print(Fiber.new {
  file.writeBytes("one")
}.try()) // expect: File is not open.

File.delete("file.temp")
