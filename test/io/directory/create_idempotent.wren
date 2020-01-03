import "io" for Directory

// given
System.print(Directory.exists("test1")) // expect: false

// when creaate is idempotent
System.print(Directory.create("test1")) // expect: true
System.print(Directory.create("test1")) // expect: false

// then
System.print(Directory.exists("test1")) // expect: true

Directory.delete("test1")

