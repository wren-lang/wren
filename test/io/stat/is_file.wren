import "io" for Stat

System.print(Stat.path("test/io/file/file.txt").isFile) // expect: true
System.print(Stat.path("test").isFile) // expect: false
