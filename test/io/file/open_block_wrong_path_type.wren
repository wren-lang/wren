import "io" for File

File.open(123) {|file|} // expect runtime error: Path must be a string.
