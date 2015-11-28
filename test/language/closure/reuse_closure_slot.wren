{
  var f = null

  {
    var a = "a"
    f = fn () { System.print(a) }
  }

  {
    // Since a is out of scope, the local slot will be reused by b. Make sure
    // that f still closes over a.
    var b = "b"
    f() // expect: a
  }
}

// TODO: Maximum number of closed-over variables (directly and/or indirect).
