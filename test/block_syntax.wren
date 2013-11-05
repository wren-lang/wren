// Single line.
{ io.write("ok") }.call // expect: ok

// No trailing newline.
{
  io.write("ok") }.call // expect: ok

// Trailing newline.
{
  io.write("ok") // expect: ok
}.call

// Multiple expressions.
{
  io.write("1") // expect: 1
  io.write("2") // expect: 2
}.call

// Extra newlines.
{


  io.write("1") // expect: 1


  io.write("2") // expect: 2


}.call

// TODO(bob): Arguments.
