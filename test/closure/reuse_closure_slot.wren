{
  var f = null

  {
    var a = "a"
    f = fn io.write(a)
  }

  {
    // Since a is out of scope, the local slot will be reused by b. Make sure
    // that f still closes over a.
    var b = "b"
    f.call // expect: a
  }
}

// TODO(bob): Closing over this.
// TODO(bob): Close over fn/method parameter.
// TODO(bob): Maximum number of closed-over variables (directly and/or indirect).
