var f = Fn.new {
  System.print(Global)
}

var Global = "global"

f() // expect: global