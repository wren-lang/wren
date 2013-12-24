IO.write(fn {
  if (false) "no" else return "ok"
}.call) // expect: ok
