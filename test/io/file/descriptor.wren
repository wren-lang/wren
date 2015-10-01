import "io" for File

var file = File.open("test/io/file/descriptor.wren")

// We can't test for a specific value since it's up to the OS, but it should be
// a positive number.
System.print(file.descriptor is Num) // expect: true
System.print(file.descriptor > 0) // expect: true

file.close()
System.print(file.descriptor) // expect: -1
