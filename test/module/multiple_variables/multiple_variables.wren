import "module" for Module1, Module2, Module3, Module4, Module5

// Only execute module body once:
// expect: ran module

IO.print(Module1) // expect: from module one
IO.print(Module2) // expect: from module two
IO.print(Module3) // expect: from module three
IO.print(Module4) // expect: from module four
IO.print(Module5) // expect: from module five
