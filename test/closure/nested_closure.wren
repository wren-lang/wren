var f = null

new Fn {
  var a = "a"
  new Fn {
    var b = "b"
    new Fn {
      var c = "c"
      f = new Fn {
        IO.print(a)
        IO.print(b)
        IO.print(c)
      }
    }.call()
  }.call()
}.call()

f.call()
// expect: a
// expect: b
// expect: c
