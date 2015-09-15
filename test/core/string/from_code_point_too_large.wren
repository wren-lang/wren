// UTF-8 mandates that only values up to 10ffff can be encoded.
// See: http://tools.ietf.org/html/rfc3629
System.print(String.fromCodePoint(0x10ffff + 1))   // expect runtime error: Code point cannot be greater than 0x10ffff.
