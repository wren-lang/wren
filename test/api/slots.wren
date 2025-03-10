class Slots {
  foreign static noSet
  foreign static getSlots(bool, num, string, bytes, value)
  foreign static setSlots(a, b, c, d, e)
  foreign static slotTypes(bool, foreignObj, list, map, nullObj, num, string, unknown)
  foreign static ensure()
  foreign static ensureOutsideForeign()
  foreign static getListCount(list)
  foreign static getListElement(list, index)
  foreign static getMapValue(map, key)
  foreign static getSlotClass(obj)
  foreign static getSlotClassName(obj)
  foreign static isParameterForeignType(param)
  foreign static isParameterForeignTypeByName(param)
}

foreign class ForeignType {
  construct new() {}
}

// If nothing is set in the return slot, it retains its previous value, the
// receiver.
System.print(Slots.noSet == Slots) // expect: true

var value = ["value"]
System.print(Slots.getSlots(true, "by\0te", 1.5, "str", value) == value)
// expect: true

System.print(Slots.setSlots(value, 0, 0, 0, 0) == value)
// expect: true

System.print(Slots.slotTypes(false, ForeignType.new(), [], {}, null, 1.2, "str", 1..2))
// expect: true

System.print(Slots.ensure())
// expect: 1 -> 20 (190)

System.print(Slots.ensureOutsideForeign())
// expect: 0 -> 20 (190)

var ducks = ["Huey", "Dewey", "Louie"]
System.print(Slots.getListCount(ducks))      // expect: 3
System.print(Slots.getListElement(ducks, 0)) // expect: Huey
System.print(Slots.getListElement(ducks, 1)) // expect: Dewey

var capitals = {
  "England": "London",
  "Scotland": "Edinburgh",
  "Wales": "Cardiff",
  "N. Ireland": "Belfast"
}

System.print(Slots.getMapValue(capitals, "England")) // expect: London
System.print(Slots.getMapValue(capitals, "Wales")) // expect: Cardiff
System.print(Slots.getMapValue(capitals, "S. Ireland")) // expect: null

System.print(Slots.getSlotClass(Slots)) // expect: Slots
System.print(Slots.getSlotClass(ForeignType)) // expect: ForeignType
System.print(Slots.getSlotClass(ForeignType.new())) // expect: ForeignType
System.print(Slots.getSlotClass(capitals)) // expect: Map
System.print(Slots.getSlotClass([])) // expect: List

// If a class is given, returns the name of the class.
// If anything but a class is given, returns null.
System.print(Slots.getSlotClassName(Class)) // expect: Class
System.print(Slots.getSlotClassName(Slots)) // expect: Slots
System.print(Slots.getSlotClassName(Map)) // expect: Map
System.print(Slots.getSlotClassName(capitals)) // expect: null
System.print(Slots.getSlotClassName(Slots.getSlotClass(capitals)))
// expect: Map
System.print(Slots.getSlotClassName(ducks)) // expect: null
System.print(Slots.getSlotClassName(Slots.getSlotClass(ducks))) // expect: List
System.print(Slots.getSlotClassName(Bool)) // expect: Bool
System.print(Slots.getSlotClassName(true)) // expect: null
System.print(Slots.getSlotClassName(Slots.getSlotClass(true))) // expect: Bool
System.print(Slots.getSlotClassName(Null)) // expect: Null
// The "null" is misleading as one might think it returns the value it
// has been given, but this is correct behavior as the given "null" is not
// a class, but instead a class instance: it is the second case above.
System.print(Slots.getSlotClassName(null)) // expect: null
System.print(Slots.getSlotClassName(Slots.getSlotClass(null))) // expect: Null

System.print(Slots.isParameterForeignType(Slots)) // expect: false
System.print(Slots.isParameterForeignType(ForeignType)) // expect: true
System.print(Slots.isParameterForeignType(ForeignType.new())) // expect: true
System.print(Slots.isParameterForeignType(Bool)) // expect: false
System.print(Slots.isParameterForeignType(true)) // expect: false

System.print(Slots.isParameterForeignTypeByName(Slots)) // expect: false
System.print(Slots.isParameterForeignTypeByName(ForeignType)) // expect: true
System.print(Slots.isParameterForeignTypeByName(ForeignType.new()))
// expect: true
System.print(Slots.isParameterForeignTypeByName(Bool)) // expect: false
System.print(Slots.isParameterForeignTypeByName(true)) // expect: false
