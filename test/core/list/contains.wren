var list = [1, 2, 3, 4, "foo"]

System.print(list.contains(2))      // expect: true
System.print(list.contains(5))      // expect: false
System.print(list.contains("foo"))  // expect: true
System.print(list.contains("bar"))  // expect: false
