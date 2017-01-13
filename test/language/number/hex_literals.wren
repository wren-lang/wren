var x = 0xFF

System.print(x) // expect: 255
System.print(x + 1) // expect: 256
System.print(x == 255) // expect: true
System.print(0x09 is Num) // expect: true
System.print(x is Num) // expect: true
System.print(-0xFF) // expect: -255
System.print(0xdeadbeef) // expect: 3735928559
