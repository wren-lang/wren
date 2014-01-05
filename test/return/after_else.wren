IO.print(fn {
  if (false) "no" else return "ok"
}.call) // expect: ok
