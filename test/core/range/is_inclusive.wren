System.print((0..0).isInclusive) // expect: true
System.print((0...0).isInclusive) // expect: false

System.print((-1..1).isInclusive) // expect: true
System.print((-1...1).isInclusive) // expect: false
