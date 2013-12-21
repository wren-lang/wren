// Basic assignment.
{
  var list = [1, 2, 3]
  list[0] = 5
  list[1] = 6
  list[2] = 7
  io.write(list) // expect: [5, 6, 7]
}

// Returns right-hand side.
{
  var list = [1, 2, 3]
  io.write(list[1] = 5) // expect: 5
}

// Negative indices.
{
  var list = [1, 2, 3]
  list[-1] = 5
  list[-2] = 6
  list[-3] = 7
  io.write(list) // expect: [7, 6, 5]
}

// TODO: Handle out of range.
// TODO: Wrong non-number type.
// TODO: Floating-point subscript.
//{
//  var list = [1, 2, 3, 4]
//  list[4] = 1
//}
//
//{
//  var list = [1, 2, 3, 4]
//  list[-5] = 1
//}

// TODO: Not in this dir, but need tests for subscript setter grammar.
