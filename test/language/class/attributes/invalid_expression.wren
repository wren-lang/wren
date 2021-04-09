
// When used in an expression location, 
// the error remains Error at '#': Expected expression

#valid
class Example {

  #valid
  method() {
    return #invalid 1 // expect error
  }
}