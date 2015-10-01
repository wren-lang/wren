import "io" for File

File.open("nonexistent") {|file|} // expect runtime error: no such file or directory
