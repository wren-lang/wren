// Ported from the Wren version.

class Tree {
  var _item;
  var _left;
  var _right;

  Tree(item, depth) {
    _item = item;
    if (depth > 0) {
      var item2 = item + item;
      depth--;
      _left = new Tree(item2 - 1, depth);
      _right = new Tree(item2, depth);
    }
  }

  get check {
    if (_left == null) {
      return _item;
    }

    return _item + _left.check - _right.check;
  }
}

main() {
  var minDepth = 4;
  var maxDepth = 12;
  var stretchDepth = maxDepth + 1;

  Stopwatch watch = new Stopwatch();
  watch.start();

  print("stretch tree of depth ${stretchDepth} check: "
      "${new Tree(0, stretchDepth).check}");

  var longLivedTree = new Tree(0, maxDepth);

  // iterations = 2 ** maxDepth
  var iterations = 1;
  for (var d = 0; d < maxDepth; d++) {
    iterations = iterations * 2;
  }

  var depth = minDepth;
  while (depth < stretchDepth) {
    var check = 0;
    for (var i = 1; i <= iterations; i++) {
      check += new Tree(i, depth).check + new Tree(-i, depth).check;
    }

    print("${iterations * 2} trees of depth ${depth} check: ${check}");
    iterations ~/= 4;
    depth += 2;
  }

  print("long lived tree of depth ${maxDepth} check: ${longLivedTree.check}");
  print("elapsed: ${watch.elapsedMilliseconds / 1000}");
}
