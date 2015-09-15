// This mostly tests that lists handle growing and shrinking their memory.
var list = []
for (i in 0..200) {
  list.add(i)
}

for (i in 0..195) {
  list.removeAt(-1)
}

System.print(list) // expect: [0, 1, 2, 3, 4]
