// With no attributes defined, no ClassAttributes should be allocated

class Without {}
System.print(Without.attributes == null)    // expect: true
