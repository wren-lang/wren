
var tracingIterate = Fn.new {|iter, seq|
  iter = seq.iterate(iter)
  System.print(seq.iteratorValue(iter))
  return iter
}

for (i in 0..5, , tracingIterate) {
  System.print(i)
}

// expect: 0
// expect: 0
// expect: 1
// expect: 1
// expect: 2
// expect: 2
// expect: 3
// expect: 3
// expect: 4
// expect: 4
// expect: 5
// expect: 5
// expect: false
