import "io" for File

var file = File.open("test/io/file/is_open.wren")
System.print(file.isOpen) // expect: true
file.close()
System.print(file.isOpen) // expect: false
