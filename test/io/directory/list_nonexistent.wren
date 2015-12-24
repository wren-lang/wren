import "io" for Directory

Directory.list("nonexistent") // expect runtime error: no such file or directory
