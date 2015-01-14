var Global = "global"
// TODO: Forward reference to global declared after use.

new Fn {
  IO.print(Global) // expect: global
}.call
