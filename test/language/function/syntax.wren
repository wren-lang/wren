// Single expression body.
Fn.new { IO.print("ok") }.call() // expect: ok

// Curly body.
Fn.new {
  IO.print("ok") // expect: ok
}.call()

// Multiple statements.
Fn.new {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call()

// Extra newlines.
Fn.new {


  IO.print("1") // expect: 1


  IO.print("2") // expect: 2


}.call()
