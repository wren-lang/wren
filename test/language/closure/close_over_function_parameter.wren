var f = null

Fn.new {|param|
  f = Fn.new {
    System.print(param)
  }
}.call("param")

f.call() // expect: param
