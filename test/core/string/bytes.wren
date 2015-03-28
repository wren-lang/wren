// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var s = "søméஃthîng"

IO.print(s.bytes is StringByteSequence) // expect: true
