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
