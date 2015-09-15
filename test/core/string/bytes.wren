// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var s = "søméஃthîng"

System.print(s.bytes is StringByteSequence) // expect: true
