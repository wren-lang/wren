var f = null

Fn.new {|param|
  f = Fn.new {
    IO.print(param)
  }
}.call("param")

f.call() // expect: param
