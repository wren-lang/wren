fn block(arg) { arg }

// Single expression body.
System.print(block { "ok" }()) // expect: ok

// Curly body.
block {
  System.print("ok") // expect: ok
}()

// Multiple statements.
block {
  System.print("1") // expect: 1
  System.print("2") // expect: 2
}()

// Extra newlines.
block {


  System.print("1") // expect: 1


  System.print("2") // expect: 2


}()
