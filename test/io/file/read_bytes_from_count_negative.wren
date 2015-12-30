import "io" for File

var file = File.open("test/io/file/file.txt")
file.readBytes(-1, 0) // expect runtime error: Count cannot be negative.
