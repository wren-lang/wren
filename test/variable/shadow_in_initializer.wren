// skip: Variables should not be in scope in their initializer.
{
  var a = "outer"
  {
    var a = a + " inner"
    io.write(a) // expect: outer inner
  }
}

// TODO: Test referring to class name inside class for class at global scope
// and in local scope.
