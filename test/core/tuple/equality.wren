var tuple0 = Tuple.new()
var tuple1 = Tuple.new(0)
var tuple2 = Tuple.new(0, 1)
var tuple3 = Tuple.new(0, 1, 2)

System.print(tuple0 == tuple0) // expect: true
System.print(tuple0 == tuple3) // expect: false
System.print(tuple1 == tuple3) // expect: false
System.print(tuple2 == tuple3) // expect: false
System.print(tuple3 == tuple3) // expect: true

System.print(tuple3 == Tuple.new(0, 1, 2)) // expect: true

// Not equal to other types.
System.print(tuple0 == []) // expect: false
System.print(tuple1 == 1)  // expect: false

// Don't test all the path since `lhs != rhs` is trivially `!(lhs == rhs)`
System.print(tuple0 != tuple0) // expect: false
