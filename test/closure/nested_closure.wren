var f = null

fn {
  var a = "a"
  fn {
    var b = "b"
    fn {
      var c = "c"
      f = fn {
        IO.print(a)
        IO.print(b)
        IO.print(c)
      }
    }.call
  }.call
}.call

f.call
// expect: a
// expect: b
// expect: c
