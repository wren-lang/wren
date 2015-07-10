var f = Fn.new {
  IO.print(Global)
}

var Global = "global"

f.call() // expect: global