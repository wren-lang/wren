{
  // Error is on last line because this assumes "a" is an implicitly defined
  // global until it reaches the end of the file.
  var a = a + 1 // expect error line 7
  System.print(a)
}
