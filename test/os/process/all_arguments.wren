import "os" for Process

var args = Process.allArguments
System.print(args is List) // expect: true
System.print(args.count)   // expect: 2

// Includes wren executable and file being run.
System.print(args[0].contains("wren")) // expect: true
System.print(args[1]) // expect: test/os/process/all_arguments.wren

// TODO: Test passing additional args once we have an API to spawn a process.
