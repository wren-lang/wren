{
  var local = "local"
  Fn.new {
    IO.print(local) // expect: local
  }.call()
}
