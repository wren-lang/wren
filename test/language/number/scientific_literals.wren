var x = 2.55e2

System.print(x) // expect: 255
System.print(x + 1) // expect: 256
System.print(x == 255) // expect: true
System.print(2.55e-2 is Num) // expect: true
System.print(x is Num) // expect: true
System.print(-2.55e2) // expect: -255
System.print(-25500e-2) // expect: -255
System.print(2.55e+2) // expect: 255
