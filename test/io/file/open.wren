import "io" for File

var file = File.open("test/io/file/open.wren")
System.print(file is File) // expect: true
System.print(file.isOpen) // expect: true
