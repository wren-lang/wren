var A = "a".import_("A")
var B = "b".import_("B")

// Shared module should only run once:
// expect: a
// expect: shared
// expect: a done
// expect: b
// expect: b done

IO.print(A) // expect: a shared
IO.print(B) // expect: b shared
