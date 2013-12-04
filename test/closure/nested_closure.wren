var f = null

fn {
  var a = "a"
  fn {
    var b = "b"
    fn {
      var c = "c"
      f = fn {
        io.write(a)
        io.write(b)
        io.write(c)
      }
    }.call
  }.call
}.call

f.call
// expect: a
// expect: b
// expect: c
