class Api {
  foreign static returnInt
  foreign static returnFloat
}

IO.print(Api.returnInt)  // expect: 123456
IO.print(Api.returnFloat) // expect: 123.456
