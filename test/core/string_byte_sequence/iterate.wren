// Bytes:
//        012345678
// Chars: sø mé ஃ
var bytes = "søméஃ".bytes

IO.print(bytes.iterate(null)) // expect: 0
IO.print("".bytes.iterate(null)) // expect: false

IO.print(bytes.iterate(0)) // expect: 1
IO.print(bytes.iterate(1)) // expect: 2
IO.print(bytes.iterate(2)) // expect: 3
IO.print(bytes.iterate(3)) // expect: 4
IO.print(bytes.iterate(4)) // expect: 5
IO.print(bytes.iterate(5)) // expect: 6
IO.print(bytes.iterate(6)) // expect: 7
IO.print(bytes.iterate(7)) // expect: 8
IO.print(bytes.iterate(8)) // expect: false

// Out of bounds.
IO.print(bytes.iterate(123)) // expect: false
IO.print(bytes.iterate(-1)) // expect: false
