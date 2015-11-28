// Single expression body.
System.print(fn () { "ok" }()) // expect: ok

// Curly body.
fn () {
  System.print("ok") // expect: ok
}()

// Multiple statements.
fn () {
  System.print("1") // expect: 1
  System.print("2") // expect: 2
}()

// Extra newlines.
fn () {


  System.print("1") // expect: 1


  System.print("2") // expect: 2


}()
