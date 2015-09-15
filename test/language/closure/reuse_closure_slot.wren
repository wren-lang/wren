{
  var f = null

  {
    var a = "a"
    f = Fn.new { System.print(a) }
  }

  {
    // Since a is out of scope, the local slot will be reused by b. Make sure
    // that f still closes over a.
    var b = "b"
    f.call() // expect: a
  }
}

// TODO: Maximum number of closed-over variables (directly and/or indirect).
