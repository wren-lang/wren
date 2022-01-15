
var double = Fn.new {|num| num * 2 }
var tracingIterate = Fn.new {|iter, seq|
  iter = seq.iterate(iter)
  System.print(seq.iteratorValue(iter))
  return iter
}

for (i in 0..5, double, tracingIterate) {
  System.print(i)
}

// expect: 0
// expect: 0
// expect: 1
// expect: 2
// expect: 2
// expect: 4
// expect: 3
// expect: 6
// expect: 4
// expect: 8
// expect: 5
// expect: 10
// expect: false
