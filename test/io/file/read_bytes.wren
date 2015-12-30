import "io" for File

var file = File.open("test/io/file/file.txt")

System.print(file.readBytes(3)) // expect: thi

// Always reads from the beginning.
System.print(file.readBytes(7)) // expect: this is

// Allows zero.
System.print(file.readBytes(0).bytes.count) // expect: 0

// A longer number reads the whole file.
System.print(file.readBytes(100)) // expect: this is a text file

// Reading past the end truncates the buffer.
System.print(file.readBytes(100).bytes.count) // expect: 19

file.close()
