// Single expression body.
(fn io.write("ok")).call // expect: ok

// TODO: Precedence of fn body.

// Curly body.
fn {
  io.write("ok") // expect: ok
}.call

// No trailing newline.
fn {
  io.write("ok") }.call // expect: ok

// Multiple expressions.
fn {
  io.write("1") // expect: 1
  io.write("2") // expect: 2
}.call

// Extra newlines.
fn {


  io.write("1") // expect: 1


  io.write("2") // expect: 2


}.call
