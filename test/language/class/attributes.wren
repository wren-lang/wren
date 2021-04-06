
// No attributes should have no ClassAttributes allocated

class Without {}
System.print(Without.attributes == null)    // expect: true


// Test the basic states. Keys without a group
// go into a group with null as the key

#key
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

// Keys must be a name, and values can be any literal value

#name = name
#string = "string"
#integer = 32
#number = 2.5
#bool = true
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

// Duplicate keys add multiple values to 
// the attribute's key, in parse order 
#key
#key = value
#key=other
class DuplicateKeys {}

var dupeGroup = DuplicateKeys.attributes.self[null]
System.print(dupeGroup.count)               // expect: 1
System.print(dupeGroup["key"].count)        // expect: 3
System.print(dupeGroup["key"])              // expect: [null, value, other]

// Groups store attributes by named group

#key //not combined
#group(key=combined)
#group(key=value, key=2, key=false)
class GroupedKeys {}

var ungroupedKeys = GroupedKeys.attributes.self[null]
var groupedKeys = GroupedKeys.attributes.self["group"]

System.print(ungroupedKeys.count)               // expect: 1
System.print(groupedKeys.count)                 // expect: 1
System.print(ungroupedKeys.containsKey("key"))  // expect: true
var groupedKey = groupedKeys["key"]
System.print(groupedKey.count)                  // expect: 4
System.print(groupedKey)                        // expect: [combined, value, 2, false]


class Methods {

  #getter
  method {}

  method() {}

  #regular = 2
  #group(key, other=value, string="hello")
  method(arg0, arg1) {}

  #is_static = true
  static method() {}

}

var methodAttr = Methods.attributes.methods
var getter = methodAttr["method"]
var none = methodAttr["method()"]
var regular = methodAttr["method(_,_)"]
var aStatic = methodAttr["static method()"]

// (Be wary of relying on map order)

System.print(getter)                        // expect: {null: {getter: [null]}}
System.print(none)                          // expect: null
System.print(regular[null])                 // expect: {regular: [2]}
System.print(regular["group"]["key"])       // expect: [null]
System.print(regular["group"]["other"])     // expect: [value]
System.print(regular["group"]["string"])    // expect: [hello]
System.print(aStatic[null])                 // expect: {is_static: [true]}
