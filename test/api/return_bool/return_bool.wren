class Api {
  foreign static returnTrue
  foreign static returnFalse
}

IO.print(Api.returnTrue)  // expect: true
IO.print(Api.returnFalse) // expect: false
