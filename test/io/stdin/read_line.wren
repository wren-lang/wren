import "io" for Stdin

System.write("> ")
System.print("1 %(Stdin.readLine())")
System.write("> ")
System.print("2 %(Stdin.readLine())")

// stdin: first
// stdin: second
// expect: > 1 first
// expect: > 2 second
