// << have higher precedence than |.
System.print(2 | 1 << 1) // expect: 2
System.print(1 << 1 | 2) // expect: 2

// << has higher precedence than &.
System.print(2 & 1 << 1) // expect: 2
System.print(1 << 1 & 2) // expect: 2

// << has higher precedence than ^.
System.print(2 ^ 1 << 1) // expect: 0
System.print(1 << 1 ^ 2) // expect: 0

// & has higher precedence than |.
System.print(1 & 1 | 2) // expect: 3
System.print(2 | 1 & 1) // expect: 3

// & has higher precedence than ^.
System.print(1 & 1 ^ 2) // expect: 3
System.print(2 ^ 1 & 1) // expect: 3

// ^ has higher precedence than |.
System.print(1 ^ 1 | 1) // expect: 1
System.print(1 | 1 ^ 1) // expect: 1
