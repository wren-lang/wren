// Keys must be a name, and values can be any literal value

#!name = name
#!string = "string"
#!integer = 32
#!number = 2.5
#!bool = true
class Literals {}

var literalGroup = Literals.attributes.self[null]

System.print(literalGroup.count)                  // expect: 5
System.print(literalGroup["string"][0] is String) // expect: true
System.print(literalGroup["string"][0])           // expect: string
System.print(literalGroup["integer"][0] is Num)   // expect: true
System.print(literalGroup["integer"][0])          // expect: 32
System.print(literalGroup["number"][0] is Num)    // expect: true
System.print(literalGroup["number"][0])           // expect: 2.5
System.print(literalGroup["bool"][0] is Bool)     // expect: true
System.print(literalGroup["bool"][0])             // expect: true
