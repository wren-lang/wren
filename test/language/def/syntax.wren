// Single expression body.
def f() { System.print("ok") } // expect: ok
f()

// Curly body.
def g() {
  System.print("ok") // expect: ok
}
g()

// Multiple statements.
def h() {
  System.print("1") // expect: 1
  System.print("2") // expect: 2
}
h()

// Extra newlines.
def i() {


  System.print("1") // expect: 1


  System.print("2") // expect: 2


}
i()
