var global = "global"
// TODO: Forward reference to global declared after use.

fn {
  IO.write(global) // expect: global
}.call
