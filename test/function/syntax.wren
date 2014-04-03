// Single expression body.
new Fn { IO.print("ok") }.call // expect: ok

// Curly body.
new Fn {
  IO.print("ok") // expect: ok
}.call

// No trailing newline.
new Fn {
  IO.print("ok") }.call // expect: ok

// Multiple expressions.
new Fn {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call

// Extra newlines.
new Fn {


  IO.print("1") // expect: 1


  IO.print("2") // expect: 2


}.call
