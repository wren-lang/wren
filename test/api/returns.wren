class Api {
  foreign static implicitNull

  foreign static returnInt
  foreign static returnFloat

  foreign static returnTrue
  foreign static returnFalse
}

IO.print(Api.implicitNull == null) // expect: true

IO.print(Api.returnInt)  // expect: 123456
IO.print(Api.returnFloat) // expect: 123.456

IO.print(Api.returnTrue)  // expect: true
IO.print(Api.returnFalse) // expect: false
