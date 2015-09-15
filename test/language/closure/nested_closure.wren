var f = null

Fn.new {
  var a = "a"
  Fn.new {
    var b = "b"
    Fn.new {
      var c = "c"
      f = Fn.new {
        System.print(a)
        System.print(b)
        System.print(c)
      }
    }.call()
  }.call()
}.call()

f.call()
// expect: a
// expect: b
// expect: c
