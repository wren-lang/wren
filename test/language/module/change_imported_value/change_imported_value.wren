import "./module" for Module, Other

System.print(Module) // expect: before

// Reassigning the variable in the other module does not affect this one's
// binding.
Other.change
System.print(Module) // expect: before

// But it does change there.
Other.show // expect: after
