IO.print((0..0).isInclusive) // expect: true
IO.print((0...0).isInclusive) // expect: false

IO.print((-1..1).isInclusive) // expect: true
IO.print((-1...1).isInclusive) // expect: false
