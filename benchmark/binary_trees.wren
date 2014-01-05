// Ported from the Python version.

class Tree {
  new(item, depth) {
    _item = item
    if (depth > 0) {
      var item2 = item + item
      depth = depth - 1
      _left = new Tree(item2 - 1, depth)
      _right = new Tree(item2, depth)
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
var maxDepth = 12
var stretchDepth = maxDepth + 1

var start = OS.clock

IO.print("stretch tree of depth " + stretchDepth.toString + " check: " +
    new Tree(0, stretchDepth).check.toString)

var longLivedTree = new Tree(0, maxDepth)

// iterations = 2 ** maxDepth
var iterations = 1
for (d in 0...maxDepth) {
  iterations = iterations * 2
}

var depth = minDepth
while (depth < stretchDepth) {
  var check = 0
  var i = 1
  while (i < iterations + 1) {
    check = check + new Tree(i, depth).check + new Tree(-i, depth).check
    i = i + 1
  }

  IO.print((iterations * 2).toString + " trees of depth " + depth.toString +
      " check: " + check.toString)
  iterations = iterations / 4
  depth = depth + 2
}

IO.print("long lived tree of depth " + maxDepth.toString + " check: " +
    longLivedTree.check.toString)

IO.print("elapsed: " + (OS.clock - start).toString)
