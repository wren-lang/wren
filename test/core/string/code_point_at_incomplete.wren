// The first two bytes of a three-octet sequence.
var s = "\xe0\xae"
IO.print(s.codePointAt(0)) // expect: -1
