// In middle of line.
IO.write/* ... */(/* */"ok"/* */) // expect: ok

// Nested.
IO.write(/* in /* nest */ out */"ok") // expect: ok
