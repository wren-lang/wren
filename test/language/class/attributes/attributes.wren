// Test the basic states. Keys without a group
// go into a group with null as the key

#!key
class Attr {}

System.print(Attr.attributes != null)         // expect: true
System.print(Attr.attributes.self != null)    // expect: true
System.print(Attr.attributes.methods)         // expect: null

var attr = Attr.attributes.self
var nullGroup = attr[null]
System.print(nullGroup != null)             // expect: true
System.print(nullGroup.count)               // expect: 1
System.print(nullGroup.containsKey("key"))  // expect: true

var keyItems = nullGroup["key"]
System.print(keyItems != null)              // expect: true
System.print(keyItems is List)              // expect: true
System.print(keyItems.count)                // expect: 1
System.print(keyItems[0])                   // expect: null
