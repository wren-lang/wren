import "io" for File, Stat

// Create a file and don't close it.
var file = File.create("file.temp")

File.delete("file.temp")

file.close()

// TODO: More graceful API to tell if a file exists.
var error = Fiber.new {
  Stat.path("file.temp")
}.try()
System.print(error) // expect: no such file or directory
