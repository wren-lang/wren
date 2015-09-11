// Bytes:           11111
//        012345678901234
// Chars: sø mé ஃ  thî ng
var codePoints = "søméஃthîng".codePoints

IO.print(codePoints[0]) // expect: 115
IO.print(codePoints[1]) // expect: 248
IO.print(codePoints[2]) // expect: -1
IO.print(codePoints[3]) // expect: 109
IO.print(codePoints[4]) // expect: 233
IO.print(codePoints[5]) // expect: -1
IO.print(codePoints[6]) // expect: 2947
IO.print(codePoints[7]) // expect: -1
IO.print(codePoints[8]) // expect: -1
IO.print(codePoints[9]) // expect: 116
IO.print(codePoints[10]) // expect: 104
IO.print(codePoints[11]) // expect: 238
IO.print(codePoints[12]) // expect: -1
IO.print(codePoints[13]) // expect: 110
IO.print(codePoints[14]) // expect: 103

IO.print(codePoints[-15]) // expect: 115
IO.print(codePoints[-14]) // expect: 248
IO.print(codePoints[-13]) // expect: -1
IO.print(codePoints[-12]) // expect: 109
IO.print(codePoints[-11]) // expect: 233
IO.print(codePoints[-10]) // expect: -1
IO.print(codePoints[-9]) // expect: 2947
IO.print(codePoints[-8]) // expect: -1
IO.print(codePoints[-7]) // expect: -1
IO.print(codePoints[-6]) // expect: 116
IO.print(codePoints[-5]) // expect: 104
IO.print(codePoints[-4]) // expect: 238
IO.print(codePoints[-3]) // expect: -1
IO.print(codePoints[-2]) // expect: 110
IO.print(codePoints[-1]) // expect: 103

IO.print("\0".codePoints[0]) // expect: 0

// Returns -1 for invalid UTF-8 sequences.
IO.print("\xef\xf7".codePoints[0]) // expect: -1
IO.print("\xef\xf7".codePoints[1]) // expect: -1
