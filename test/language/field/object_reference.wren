// This test exists mainly to make sure the GC traces instance fields.
class Node {
  construct new(left, value, right) {
    _left = left
    _value = value
    _right = right
  }

  write() {
    if (_left is Node) {
      _left.write()
    }

    System.print(_value)

    if (_right is Node) {
      _right.write()
    }
  }
}

var a = Node.new(null, "a", null)
var b = Node.new(null, "b", null)
var c = Node.new(a, "c", b)
a = null
b = null
var d = Node.new(c, "d", null)
c = null
d.write()
// expect: a
// expect: c
// expect: b
// expect: d
