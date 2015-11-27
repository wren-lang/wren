// Single expression body.
fn a() { "ok" }
System.print(a()) // expect: ok

// Curly body.
fn b() {
  System.print("ok") // expect: ok
}
b()

// Multiple statements.
fn c() {
  System.print("1") // expect: 1
  System.print("2") // expect: 2
}
c()

// Extra newlines.
fn d() {


  System.print("1") // expect: 1


  System.print("2") // expect: 2


}
d()
