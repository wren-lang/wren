var Module = "module.wren".import_("Module")
var Other = "module.wren".import_("Other")

IO.print(Module) // expect: before

// Reassigning the variable in the other module does not affect this one's
// binding.
Other.change
IO.print(Module) // expect: before

// But it does change there.
Other.show // expect: after

// TODO: Cyclic import.
