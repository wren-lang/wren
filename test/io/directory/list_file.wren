import "io" for Directory

var entries = Directory.list("test/io/directory/dir/a.txt") // expect runtime error: not a directory
