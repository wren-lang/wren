var a = [1, 2, 3]
System.print(a.all {|x| x > 1 }) // expect: false
System.print(a.all {|x| x > 0 }) // expect: true
System.print([].all {|x| false }) // expect: true

// Returns first falsey value.
System.print(a.all {|x| x < 2 ? null : false }) // expect: null

// Returns last truthy value.
System.print(a.all {|x| x }) // expect: 3