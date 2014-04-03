var global = "global"
// TODO: Forward reference to global declared after use.

new Fn {
  IO.print(global) // expect: global
}.call
