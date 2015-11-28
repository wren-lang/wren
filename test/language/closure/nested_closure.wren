var f = null

fn () {
  var a = "a"
  fn () {
    var b = "b"
    fn () {
      var c = "c"
      f = fn () {
        System.print(a)
        System.print(b)
        System.print(c)
      }
    }()
  }()
}()

f()
// expect: a
// expect: b
// expect: c
