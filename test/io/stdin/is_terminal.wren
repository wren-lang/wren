import "io" for Stdin

// The tests aren't run in a terminal.
System.print(Stdin.isTerminal) // expect: false
