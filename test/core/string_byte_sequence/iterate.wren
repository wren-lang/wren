// Bytes:
//        012345678
// Chars: sø mé ஃ
var bytes = "søméஃ".bytes

System.print(bytes.iterate(null)) // expect: 0
System.print("".bytes.iterate(null)) // expect: false

System.print(bytes.iterate(0)) // expect: 1
System.print(bytes.iterate(1)) // expect: 2
System.print(bytes.iterate(2)) // expect: 3
System.print(bytes.iterate(3)) // expect: 4
System.print(bytes.iterate(4)) // expect: 5
System.print(bytes.iterate(5)) // expect: 6
System.print(bytes.iterate(6)) // expect: 7
System.print(bytes.iterate(7)) // expect: 8
System.print(bytes.iterate(8)) // expect: false

// Out of bounds.
System.print(bytes.iterate(123)) // expect: false
System.print(bytes.iterate(-1)) // expect: false
