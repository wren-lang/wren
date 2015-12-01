{
  var a = a + 1
  System.print(a)
}
// Error on last line because it assumes the "a" in the initializer is a module
// level variable about to be shadowed by the new local.
// expect error