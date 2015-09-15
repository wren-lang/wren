class Api {
  foreign static implicitNull

  foreign static returnInt
  foreign static returnFloat

  foreign static returnTrue
  foreign static returnFalse
}

System.print(Api.implicitNull == null) // expect: true

System.print(Api.returnInt)  // expect: 123456
System.print(Api.returnFloat) // expect: 123.456

System.print(Api.returnTrue)  // expect: true
System.print(Api.returnFalse) // expect: false
