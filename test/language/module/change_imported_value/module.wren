// nontest
var Module = "before"

class Other {
  def static change {
    Module = "after"
  }

  def static show {
    System.print(Module)
  }
}
