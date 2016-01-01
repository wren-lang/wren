import "io" for File

var file = File.open("test/io/file/file.txt")
file.readBytes(1, "not num") // expect runtime error: Offset must be an integer.
