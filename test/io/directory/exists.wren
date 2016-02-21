import "io" for Directory

System.print(Directory.exists("test/io/file")) // expect: true
System.print(Directory.exists("nonexistent")) // expect: false

// Files are not directories.
System.print(Directory.exists("test/io/file/file.txt")) // expect: false

// TODO: Symlinks.
