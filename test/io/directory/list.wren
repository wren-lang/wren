import "io" for Directory

var entries = Directory.list("test/io/directory/dir")

// Ignore OS-specific dot files like ".DS_Store".
entries = entries.where {|entry| !entry.startsWith(".") }.toList

System.print(entries) // expect: [a.txt, b.txt, c.txt]
