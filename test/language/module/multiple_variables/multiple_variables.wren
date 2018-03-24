import "./module" for Module1, Module2, Module3, Module4, Module5

// Only execute module body once:
// expect: ran module

System.print(Module1) // expect: from module one
System.print(Module2) // expect: from module two
System.print(Module3) // expect: from module three
System.print(Module4) // expect: from module four
System.print(Module5) // expect: from module five
