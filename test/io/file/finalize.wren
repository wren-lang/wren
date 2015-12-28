import "io" for File

// Don't store in a variable.
File.open("test/io/file/finalize.wren")

System.gc()
// We can't really test what the finalizer *does* from Wren, since the object
// is unreachable, but this at least ensures it doesn't crash.

System.print("ok") // expect: ok