// skip: Variables should not be in scope in their initializer.
{
  var a = a + 1 // expect error
  io.write(a)
}
