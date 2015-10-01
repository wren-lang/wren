import "io" for File

File.size("nonexistent") // expect runtime error: no such file or directory
