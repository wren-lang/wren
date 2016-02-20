import "io" for File

File.delete("nonexistent") // expect runtime error: no such file or directory
