import "io" for File, Stat

// Create a file.
var file = File.create("file.temp")
file.close()

File.delete("file.temp")

// TODO: More graceful API to tell if a file exists.
var error = Fiber.new {
  Stat.path("file.temp")
}.try()
System.print(error) // expect: no such file or directory
