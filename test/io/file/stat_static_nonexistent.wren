import "io" for File

File.stat("nonexistent") // expect runtime error: no such file or directory
