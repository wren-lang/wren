var f = null

new Fn {|param|
  f = new Fn {
    IO.print(param)
  }
}.call("param")

f.call() // expect: param
