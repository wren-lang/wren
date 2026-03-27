// 0xFFFFFFFFFFFFFFFF overflows signed strtoll; must parse as unsigned (strtoull).
System.print(0xFFFFFFFFFFFFFFFF) // expect: 1.844674407371e+19
