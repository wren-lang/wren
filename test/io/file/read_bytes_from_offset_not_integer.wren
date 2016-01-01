import "io" for File

var file = File.open("test/io/file/file.txt")
file.readBytes(1, 1.2) // expect runtime error: Offset must be an integer.
