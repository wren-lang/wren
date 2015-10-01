import "io" for File

var file = File.open("test/io/file/file.txt")
file.close()

file.readBytes(3) // expect runtime error: File is not open.
