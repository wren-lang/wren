System.print(fn () {
  if (false) "no" else return "ok"
}()) // expect: ok
