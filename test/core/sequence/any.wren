var a = [1, 2, 3]
System.print(a.any {|x| x > 3 }) // expect: false
System.print(a.any {|x| x > 1 }) // expect: true
System.print([].any {|x| true }) // expect: false

// Returns first truthy value.
System.print(a.any {|x| x }) // expect: 1

// Returns last falsey value.
System.print(a.any {|x| x < 2 ? null : false }) // expect: false
