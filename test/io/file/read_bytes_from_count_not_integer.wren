import "io" for File

var file = File.open("test/io/file/file.txt")
file.readBytes(1.2, 0) // expect runtime error: Count must be an integer.
