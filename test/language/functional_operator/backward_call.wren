var add1   = Fn.new {|x| x + 1}
var double = Fn.new {|x| x * 2}

System.print(double |< 1) // expect: 2

// Check operator priority
System.print(double |< add1 |< 1) // expect: 4

// Swallow a trailing newline.
System.print(double |<
    1) // expect: 2
