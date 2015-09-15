// Bytes:
//        012345678
// Chars: sø mé ஃ
var bytes = "søméஃ".bytes

System.print(bytes.iteratorValue(0)) // expect: 115
System.print(bytes.iteratorValue(1)) // expect: 195
System.print(bytes.iteratorValue(2)) // expect: 184
System.print(bytes.iteratorValue(3)) // expect: 109
System.print(bytes.iteratorValue(4)) // expect: 195
System.print(bytes.iteratorValue(5)) // expect: 169
System.print(bytes.iteratorValue(6)) // expect: 224
System.print(bytes.iteratorValue(7)) // expect: 174
System.print(bytes.iteratorValue(8)) // expect: 131

System.print(bytes.iteratorValue(-9)) // expect: 115
System.print(bytes.iteratorValue(-8)) // expect: 195
System.print(bytes.iteratorValue(-7)) // expect: 184
System.print(bytes.iteratorValue(-6)) // expect: 109
System.print(bytes.iteratorValue(-5)) // expect: 195
System.print(bytes.iteratorValue(-4)) // expect: 169
System.print(bytes.iteratorValue(-3)) // expect: 224
System.print(bytes.iteratorValue(-2)) // expect: 174
System.print(bytes.iteratorValue(-1)) // expect: 131
