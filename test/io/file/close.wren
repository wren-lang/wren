import "io" for File
import "scheduler" for Scheduler

// See also: is_open.wren.

var file = File.open("test/io/file/close.wren")

System.print(file.close()) // expect: null

// Can call multiple times.
file.close()

// If already closed, returns synchronously.
Scheduler.add {
  System.print("does not print")
}

file.close()
System.print("sync") // expect: sync
