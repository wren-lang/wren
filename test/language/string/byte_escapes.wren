var bytes = "\x00\x12\x34\x56\x78\xab\xCD\xfFf".bytes

System.print(bytes[0]) // expect: 0
System.print(bytes[1]) // expect: 18
System.print(bytes[2]) // expect: 52
System.print(bytes[3]) // expect: 86
System.print(bytes[4]) // expect: 120
System.print(bytes[5]) // expect: 171
System.print(bytes[6]) // expect: 205
System.print(bytes[7]) // expect: 255
// "f".
System.print(bytes[8]) // expect: 102
