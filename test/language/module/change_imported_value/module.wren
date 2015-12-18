// nontest
var Module = "before"

class Other {
  static def change {
    Module = "after"
  }

  static def show {
    System.print(Module)
  }
}
