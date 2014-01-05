var f = null

fn(param) {
  f = fn {
    IO.print(param)
  }
}.call("param")

f.call // expect: param
