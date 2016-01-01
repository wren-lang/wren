import "io" for File
import "scheduler" for Scheduler

System.print(File.size("test/io/file/file.txt")) // expect: 19

// Runs asynchronously.
Scheduler.add {
  System.print("async")
}

System.print(File.size("test/io/file/file.txt"))
// expect: async
// expect: 19
