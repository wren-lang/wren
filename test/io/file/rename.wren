import "io" for File

// given a new file.
System.print(File.exists("file1.temp")) // expect: false
System.print(File.exists("file2.temp")) // expect: false
var file = File.create("file1.temp")
file.writeBytes("stuff")
file.close()

// when renamed
File.rename("file1.temp", "file2.temp")

// then the file only exists under the old name
System.print(File.exists("file1.temp")) // expect: false
System.print(File.exists("file2.temp")) // expect: true

File.delete("file2.temp")
