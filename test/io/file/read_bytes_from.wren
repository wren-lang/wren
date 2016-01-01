import "io" for File

var file = File.open("test/io/file/file.txt")

// Zero starts at the beginning.
System.print(file.readBytes(3, 0)) // expect: thi

// Starts at the offset.
System.print(file.readBytes(8, 3)) // expect: s is a t

// Allows zero.
System.print(file.readBytes(0, 4).bytes.count) // expect: 0

// A longer number length reads until the end.
System.print(file.readBytes(100, 2)) // expect: is is a text file

// An offset past the end returns an empty string.
System.print(file.readBytes(100, 30).bytes.count) // expect: 0

file.close()
