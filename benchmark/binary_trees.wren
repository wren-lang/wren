// Ported from the Python version.

class Tree {
  this new(item, depth) {
    _item = item
    if (depth > 0) {
      var item2 = item + item
      depth = depth - 1
      _left = Tree.new(item2 - 1, depth)
      _right = Tree.new(item2, depth)
    }
  }

  check {
    if (_left == null) {
      return _item
    }

    return _item + _left.check - _right.check
  }
}

var minDepth = 4
var maxDepth = 14
var stretchDepth = maxDepth + 1

var start = OS.clock

io.write("stretch tree of depth " + stretchDepth.toString + "\t check: " +
    Tree.new(0, stretchDepth).check.toString)

var longLivedTree = Tree.new(0, maxDepth)

// iterations = 2 ** maxDepth
var iterations = 1
var d = 0
while (d < maxDepth) {
  iterations = iterations * 2
  d = d + 1
}

var depth = minDepth
while (depth < stretchDepth) {
  var check = 0
  var i = 1
  while (i < iterations + 1) {
    check = check + Tree.new(i, depth).check + Tree.new(-i, depth).check
    i = i + 1
  }

  io.write((iterations * 2).toString + "\t trees of depth " + depth.toString +
      "\t check: " + check.toString)
  iterations = iterations / 4
  depth = depth + 2
}

io.write("long lived tree of depth " + maxDepth.toString + "\t check: " +
    longLivedTree.check.toString)

io.write("elapsed: " + (OS.clock - start).toString)
