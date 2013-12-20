// skip: Variables should not be in scope in their initializer.
{
  var a = "outer"
  {
    var a = a + " inner"
    io.write(a) // expect: outer inner
  }
}
