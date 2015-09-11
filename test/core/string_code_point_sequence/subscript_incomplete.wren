// The first byte of a two-octet sequence.
IO.print("\xc0".codePoints[0]) // expect: -1

// The first byte of a three-octet sequence.
IO.print("\xe0".codePoints[0]) // expect: -1

// The first two bytes of a three-octet sequence.
IO.print("\xe0\xae".codePoints[0]) // expect: -1
