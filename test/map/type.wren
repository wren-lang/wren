// TODO: Use map literal.

IO.print(new Map is Map)      // expect: true
// TODO: Abstract base class for associations.
IO.print(new Map is Object)    // expect: true
IO.print(new Map is Bool)      // expect: false
IO.print((new Map).type == Map) // expect: true
