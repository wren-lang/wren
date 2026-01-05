
var double = Fn.new {|num| num * 2 }

for (i in 0..5, double) {
  System.print(i)
}

// expect: 0
// expect: 2
// expect: 4
// expect: 6
// expect: 8
// expect: 10
