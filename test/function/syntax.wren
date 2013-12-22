// Single expression body.
(fn IO.write("ok")).call // expect: ok

// TODO: Precedence of fn body.

// Curly body.
fn {
  IO.write("ok") // expect: ok
}.call

// No trailing newline.
fn {
  IO.write("ok") }.call // expect: ok

// Multiple expressions.
fn {
  IO.write("1") // expect: 1
  IO.write("2") // expect: 2
}.call

// Extra newlines.
fn {


  IO.write("1") // expect: 1


  IO.write("2") // expect: 2


}.call
