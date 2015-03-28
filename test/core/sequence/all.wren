var a = [1, 2, 3]
IO.print(a.all {|x| x > 1 }) // expect: false
IO.print(a.all {|x| x > 0 }) // expect: true
IO.print([].all {|x| false }) // expect: true

// Returns first falsey value.
IO.print(a.all {|x| x < 2 ? null : false }) // expect: null

// Returns last truthy value.
IO.print(a.all {|x| x }) // expect: 3