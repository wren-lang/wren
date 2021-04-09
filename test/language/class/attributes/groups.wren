// Groups store attributes by named group

#!key //not combined
#!group(key=combined)
#!group(key=value, key=2, key=false)
class GroupedKeys {}

var ungroupedKeys = GroupedKeys.attributes.self[null]
var groupedKeys = GroupedKeys.attributes.self["group"]

System.print(ungroupedKeys.count)               // expect: 1
System.print(groupedKeys.count)                 // expect: 1
System.print(ungroupedKeys.containsKey("key"))  // expect: true
var groupedKey = groupedKeys["key"]
System.print(groupedKey.count)                  // expect: 4
System.print(groupedKey)                        // expect: [combined, value, 2, false]

