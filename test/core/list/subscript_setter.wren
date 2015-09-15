// Basic assignment.
{
  var list = [1, 2, 3]
  list[0] = 5
  list[1] = 6
  list[2] = 7
  System.print(list) // expect: [5, 6, 7]
}

// Returns right-hand side.
{
  var list = [1, 2, 3]
  System.print(list[1] = 5) // expect: 5
}

// Negative indices.
{
  var list = [1, 2, 3]
  list[-1] = 5
  list[-2] = 6
  list[-3] = 7
  System.print(list) // expect: [7, 6, 5]
}
