// The first byte of a two-octet sequence.
IO.print("\xc0".codePointAt(0)) // expect: -1

// The first byte of a three-octet sequence.
IO.print("\xe0".codePointAt(0)) // expect: -1

// The first two bytes of a three-octet sequence.
IO.print("\xe0\xae".codePointAt(0)) // expect: -1
