var global = "global"
// TODO: Forward reference to global declared after use.

fn {
  io.write(global) // expect: global
}.call
