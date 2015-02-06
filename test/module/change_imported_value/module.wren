// nontest
var Module = "before"

class Other {
  static change {
    Module = "after"
  }

  static show {
    IO.print(Module)
  }
}
