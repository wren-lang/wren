import "a" for A
import "b" for B

// Shared module should only run once:
// expect: a
// expect: shared
// expect: a done
// expect: b
// expect: b done

IO.print(A) // expect: a shared
IO.print(B) // expect: b shared
