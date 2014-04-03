IO.print(new Fn {
  if (false) "no" else return "ok"
}.call) // expect: ok
