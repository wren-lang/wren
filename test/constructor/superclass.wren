class A {
  new(arg) {
    io.write("new A " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  new(arg1, arg2) {
    super(arg2)
    io.write("new B " + arg1)
    _field = arg1
  }

  bField { return _field }
}

class C is B {
  new {
    super("one", "two")
    io.write("new C")
    _field = "c"
  }

  cField { return _field }
}

var c = new C
// expect: new A two
// expect: new B one
// expect: new C
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true

io.write(c.aField) // expect: two
io.write(c.bField) // expect: one
io.write(c.cField) // expect: c
