import "io" for File
import "scheduler" for Scheduler

System.print(File.size("test/io/file/size.wren")) // expect: 270

// Runs asynchronously.
Scheduler.add {
  System.print("async")
}

System.print(File.size("test/io/file/size.wren"))
// expect: async
// expect: 270
