// TODO: Comma-separated list.
import "module" for Module1
import "module" for Module2
import "module" for Module3
import "module" for Module4
import "module" for Module5

// Only execute module body once:
// expect: ran module

IO.print(Module1) // expect: from module one
IO.print(Module2) // expect: from module two
IO.print(Module3) // expect: from module three
IO.print(Module4) // expect: from module four
IO.print(Module5) // expect: from module five
