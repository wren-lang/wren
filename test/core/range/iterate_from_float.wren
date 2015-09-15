// Starts at "from" and adds 1.0 each iteration.

for (n in 1.3..4.5) System.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
// expect: 4.3

for (n in 1.3...4.5) System.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
// expect: 4.3

for (n in 1.3...4.3) System.print(n)
// expect: 1.3
// expect: 2.3
// expect: 3.3
