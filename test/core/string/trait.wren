String.trait("answer", 42)
String.trait("world", Fn.new{|self| "%(self) World"})

System.print("Hello".trait("world")) // expect: Hello World
System.print(String.trait("answer")) // expect: 42