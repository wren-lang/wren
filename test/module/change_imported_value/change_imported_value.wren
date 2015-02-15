// TODO: Use comma-separated list.
import "module" for Module
import "module" for Other

IO.print(Module) // expect: before

// Reassigning the variable in the other module does not affect this one's
// binding.
Other.change
IO.print(Module) // expect: before

// But it does change there.
Other.show // expect: after
