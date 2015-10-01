import "io" for File

var file = File.open("test/io/file/file.txt")

System.print(file.readBytes(3)) // expect: thi

// Always reads from the beginning.
System.print(file.readBytes(7)) // expect: this is

// Allows zero.
System.print(file.readBytes(0).bytes.count) // expect: 0

file.close()
