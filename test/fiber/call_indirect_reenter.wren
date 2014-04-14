var a
var b

a = new Fiber {
  b.call // expect runtime error: Fiber has already been called.
}

b = new Fiber {
  a.call
}

b.call
