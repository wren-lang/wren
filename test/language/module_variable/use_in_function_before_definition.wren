var f = Fn.new {
  System.print(variable)
}

var variable = "module"

f.call() // expect: module