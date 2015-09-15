System.print({} is Map)      // expect: true
// TODO: Abstract base class for associations.
System.print({} is Object)    // expect: true
System.print({} is Bool)      // expect: false
System.print({}.type == Map) // expect: true
