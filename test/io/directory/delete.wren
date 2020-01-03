import "io" for Directory

// given
Directory.create("test1")

// when
Directory.delete("test1")

// then
System.print(Directory.exists("test1")) // expect: false

