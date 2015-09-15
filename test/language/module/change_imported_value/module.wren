// nontest
var Module = "before"

class Other {
  static change {
    Module = "after"
  }

  static show {
    System.print(Module)
  }
}
