// This was crashing the compiler with an out of bounds memory access.

// expect error line 6
// expect error line 7
Fiber.new {
     isDone ["", àààààààààààààààààààààààààààààààààààààààààààààààààà
