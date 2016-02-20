import "io" for File

// Create a new file.
var f
File.create("file.temp") {|file|
  f = file
  System.print(file.isOpen) // expect: true
  System.print(file.size) // expect: 0

  file.writeBytes("stuff")
}

// Closed on block exit.
System.print(f.isOpen) // expect: false

System.print(File.size("file.temp")) // expect: 5

// Overwrite a file.
File.create("file.temp") {|file|
  // Truncates.
  System.print(file.size) // expect: 0
}

File.delete("file.temp")
// TODO: Test return value.
