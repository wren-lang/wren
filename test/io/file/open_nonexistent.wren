import "io" for File

File.open("nonexistent") // expect runtime error: no such file or directory
