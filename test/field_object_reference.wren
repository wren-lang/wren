// This test exists mainly to make sure the GC traces instance fields.
class Node {
  set(left, value, right) {
    _left = left
    _value = value
    _right = right
  }

  write {
    if (_left is Node) {
      _left.write
    }

    io.write(_value)

    if (_right is Node) {
      _right.write
    }
  }
}

var a = Node.new
a.set(null, "a", null)
var b = Node.new
b.set(null, "b", null)
var c = Node.new
c.set(a, "c", b)
a = null
b = null
var d = Node.new
d.set(c, "d", null)
c = null
d.write
// expect: a
// expect: c
// expect: b
// expect: d
