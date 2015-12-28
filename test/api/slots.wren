class Slots {
  foreign static noSet
  foreign static getSlots(bool, num, string, bytes, value)
  foreign static setSlots(a, b, c, d)
  foreign static ensure()
  foreign static ensureOutsideForeign()
}

// If nothing is set in the return slot, it retains its previous value, the
// receiver.
System.print(Slots.noSet == Slots) // expect: true

var value = ["value"]
System.print(Slots.getSlots(true, "by\0te", 12.34, "str", value) == value)
// expect: true

System.print(Slots.setSlots(value, 0, 0, 0) == value)
// expect: true

System.print(Slots.ensure())
// expect: 1 -> 20 (190)

System.print(Slots.ensureOutsideForeign())
// expect: 0 -> 20 (190)
