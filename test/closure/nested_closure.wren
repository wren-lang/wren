var f = null

fn {
  var a = "a"
  fn {
    var b = "b"
    fn {
      var c = "c"
      f = fn {
        IO.write(a)
        IO.write(b)
        IO.write(c)
      }
    }.call
  }.call
}.call

f.call
// expect: a
// expect: b
// expect: c
