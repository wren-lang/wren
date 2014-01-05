var global = "global"
// TODO: Forward reference to global declared after use.

fn {
  IO.print(global) // expect: global
}.call
