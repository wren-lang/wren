import "io" for File

var text = File.read("test/io/file/file.txt")
System.print(text) // expect: this is a text file
System.print(text.count) // expect: 19

// TODO: A file containing line endings.
// TODO: A file containing null bytes.
