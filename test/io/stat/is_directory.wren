import "io" for Stat

System.print(Stat.path("test/io/file/file.txt").isDirectory) // expect: false
System.print(Stat.path("test").isDirectory) // expect: true
