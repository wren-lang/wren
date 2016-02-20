import "io" for File

var stash
File.open("test/io/file/open_block.wren") {|file|
  System.print(file is File) // expect: true
  System.print(file.isOpen) // expect: true
  stash = file
}

// Closes after block.
System.print(stash.isOpen) // expect: false

// Returns null.
System.print(File.open("test/io/file/open_block.wren") {|file|}) // expect: null

// TODO: Test a fiber aborting inside the block.
// TODO: Test return value.
