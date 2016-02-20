import "io" for File

File.create("file.temp") {|file|
  file.writeBytes("prettyshort", 0)
  file.writeBytes("agoodbitlonger", 2)
  file.writeBytes("short", 3)
}

System.print(File.read("file.temp")) // expect: prashortitlonger

File.delete("file.temp")
