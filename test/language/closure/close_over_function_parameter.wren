var f = null

fn (param) {
  f = fn () {
    System.print(param)
  }
}("param")

f() // expect: param
