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

    IO.write(_value)

    if (_right is Node) {
      _right.write
    }
  }
}

var a = new Node
a.set(null, "a", null)
var b = new Node
b.set(null, "b", null)
var c = new Node
c.set(a, "c", b)
a = null
b = null
var d = new Node
d.set(c, "d", null)
c = null
d.write
// expect: a
// expect: c
// expect: b
// expect: d
