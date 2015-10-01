import "io" for File

File.read(123) // expect runtime error: Path must be a string.
