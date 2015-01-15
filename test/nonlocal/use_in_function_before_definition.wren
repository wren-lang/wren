var f = new Fn {
  IO.print(Global)
}

var Global = "global"

f.call // expect: global