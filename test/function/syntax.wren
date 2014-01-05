// Single expression body.
(fn IO.print("ok")).call // expect: ok

// TODO: Precedence of fn body.

// Curly body.
fn {
  IO.print("ok") // expect: ok
}.call

// No trailing newline.
fn {
  IO.print("ok") }.call // expect: ok

// Multiple expressions.
fn {
  IO.print("1") // expect: 1
  IO.print("2") // expect: 2
}.call

// Extra newlines.
fn {


  IO.print("1") // expect: 1


  IO.print("2") // expect: 2


}.call
