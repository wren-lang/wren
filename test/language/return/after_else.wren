System.print(Fn.new {
  if (false) "no" else return "ok"
}.call()) // expect: ok
