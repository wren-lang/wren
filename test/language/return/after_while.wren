IO.print(new Fn {
  while (true) return "ok"
}.call()) // expect: ok
