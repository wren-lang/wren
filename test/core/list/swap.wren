var list = [0, 1, 2, 3, 4]

list.swap(0, 3) 
System.print(list) // expect: [3, 1, 2, 0, 4]

list.swap(-1, 2)
System.print(list) // expect: [3, 1, 4, 0, 2]

list.swap(8, 0) // expect runtime error: Index 0 out of bounds.