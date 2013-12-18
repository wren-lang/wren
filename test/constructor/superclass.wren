class A {
  this new(arg) {
    io.write("A.new " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  this otherName(arg1, arg2) super.new(arg2) {
    io.write("B.otherName " + arg1)
    _field = arg1
  }

  bField { return _field }
}

class C is B {
  this create super.otherName("one", "two") {
    io.write("C.create")
    _field = "c"
  }

  cField { return _field }
}

var c = C.create
// expect: A.new two
// expect: B.otherName one
// expect: C.create
io.write(c is A) // expect: true
io.write(c is B) // expect: true
io.write(c is C) // expect: true

io.write(c.aField) // expect: two
io.write(c.bField) // expect: one
io.write(c.cField) // expect: c
