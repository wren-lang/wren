var f = null

fn(param) {
  f = fn {
    io.write(param)
  }
}.call("param")

f.call // expect: param
