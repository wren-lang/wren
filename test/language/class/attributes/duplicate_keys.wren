// Duplicate keys add multiple values to 
// the attribute's key, in parse order 
#!key
#!key = value
#!key=other
class DuplicateKeys {}

var dupeGroup = DuplicateKeys.attributes.self[null]
System.print(dupeGroup.count)               // expect: 1
System.print(dupeGroup["key"].count)        // expect: 3
System.print(dupeGroup["key"])              // expect: [null, value, other]
