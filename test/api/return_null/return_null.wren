class Api {
  foreign static implicitNull
}

IO.print(Api.implicitNull == null) // expect: true
