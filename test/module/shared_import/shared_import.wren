var A = "a.wren".import_("A")
var B = "b.wren".import_("B")

// Shared module should only run once:
// expect: a
// expect: shared
// expect: a done
// expect: b
// expect: b done

IO.print(A) // expect: a shared
IO.print(B) // expect: b shared
