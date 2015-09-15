{
  var foo = "closure"
  Fn.new {
    {
      System.print(foo) // expect: closure
      var foo = "shadow"
      System.print(foo) // expect: shadow
    }
    System.print(foo) // expect: closure
  }.call()
}
