import "io" for File

System.print(File.exists("test/io/file/file.txt")) // expect: true
System.print(File.exists("nonexistent")) // expect: false

// Directories are not files.
System.print(File.exists("test/io/file")) // expect: false

// TODO: Symlinks.
