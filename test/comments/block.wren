// In middle of line.
IO.print/* ... */(/* */"ok"/* */) // expect: ok

// Nested.
IO.print(/* in /* nest */ out */"ok") // expect: ok
