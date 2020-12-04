var num = 4

System.print(num.clamp(0, 10)) // expect: 4
System.print(num.clamp(0, 1)) // expect: 1
System.print(2.clamp(0, 1)) // expect: 1
System.print((-1).clamp(0, 1)) // expect: 0
System.print((-1).clamp(-20, 0)) // expect: -1
