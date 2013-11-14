// In middle of line.
io.write/* ... */(/* */"ok"/* */) // expect: ok

// Nested.
io.write(/* in /* nest */ out */"ok") // expect: ok
