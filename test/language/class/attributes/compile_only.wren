// Attributes without a ! shouldn't be 
// passed to the runtime, they're compiled out

#compileonly
class WithNonRuntime {
  #unused
  method() {}
}
System.print(WithNonRuntime.attributes == null)    // expect: true
