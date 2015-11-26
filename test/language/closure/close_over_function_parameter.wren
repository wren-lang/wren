var f = null

Fn.new {|param|
  f = Fn.new {
    System.print(param)
  }
}("param")

f() // expect: param
