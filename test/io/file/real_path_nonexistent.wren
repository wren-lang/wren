import "io" for File

File.realPath("nonexistent") // expect runtime error: no such file or directory
