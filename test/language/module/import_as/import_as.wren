var Module = "from here"
var ValueC = "value C"
import "./module" for ValueA, Module as Another, ValueB // expect: ran module
import "./module" for ValueC as OtherC

System.print(Module)  // expect: from here
System.print(Another) // expect: from module
System.print(ValueA)  // expect: module A
System.print(ValueB)  // expect: module B
System.print(ValueC)  // expect: value C
System.print(OtherC)  // expect: module C
