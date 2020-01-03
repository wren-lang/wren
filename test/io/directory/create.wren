import "io" for Directory

// given
System.print(Directory.exists("test1")) // expect: false

// when, then
System.print(Directory.create("test1")) // expect: true

Directory.delete("test1")

