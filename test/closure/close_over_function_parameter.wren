var f = null

fn(param) {
  f = fn {
    IO.write(param)
  }
}.call("param")

f.call // expect: param
