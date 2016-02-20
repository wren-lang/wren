import "io" for File

// Create a new file.
var file = File.create("file.temp")
System.print(file.isOpen) // expect: true
System.print(file.size) // expect: 0

file.writeBytes("stuff")
file.close()

System.print(File.size("file.temp")) // expect: 5

// Overwrite a file.
file = File.create("file.temp")

// Truncates.
System.print(file.size) // expect: 0
file.close()

File.delete("file.temp")

// TODO: Test it cannot be read from.
