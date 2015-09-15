// In middle of line.
System.print/* ... */(/* */"ok"/* */) // expect: ok

// Nested.
System.print(/* in /* nest */ out */"ok") // expect: ok
