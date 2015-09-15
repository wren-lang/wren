// Bytes:
//        012345678
// Chars: sø mé ஃ
var bytes = "søméஃ".bytes

System.print(bytes[0]) // expect: 115
System.print(bytes[1]) // expect: 195
System.print(bytes[2]) // expect: 184
System.print(bytes[3]) // expect: 109
System.print(bytes[4]) // expect: 195
System.print(bytes[5]) // expect: 169
System.print(bytes[6]) // expect: 224
System.print(bytes[7]) // expect: 174
System.print(bytes[8]) // expect: 131

System.print(bytes[-9]) // expect: 115
System.print(bytes[-8]) // expect: 195
System.print(bytes[-7]) // expect: 184
System.print(bytes[-6]) // expect: 109
System.print(bytes[-5]) // expect: 195
System.print(bytes[-4]) // expect: 169
System.print(bytes[-3]) // expect: 224
System.print(bytes[-2]) // expect: 174
System.print(bytes[-1]) // expect: 131
