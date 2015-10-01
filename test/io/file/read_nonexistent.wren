import "io" for File

File.read("nonexistent") // expect runtime error: no such file or directory
