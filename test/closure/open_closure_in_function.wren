{
  var local = "local"
  fn {
    IO.write(local) // expect: local
  }.call
}
