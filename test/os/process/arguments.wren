import "os" for Process

// No additional arguments are passed to the test.
System.print(Process.arguments) // expect: []

// TODO: Test passing additional args once we have an API to spawn a process.
