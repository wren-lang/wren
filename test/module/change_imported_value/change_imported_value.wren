import "module" for Module, Other

IO.print(Module) // expect: before

// Reassigning the variable in the other module does not affect this one's
// binding.
Other.change
IO.print(Module) // expect: before

// But it does change there.
Other.show // expect: after
