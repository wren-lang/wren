import "io" for File

File.create("file.temp") {|file|
  // Appends.
  file.writeBytes("one")
  file.writeBytes("two")
  file.writeBytes("three")
}

System.print(File.read("file.temp")) // expect: onetwothree

File.delete("file.temp")
