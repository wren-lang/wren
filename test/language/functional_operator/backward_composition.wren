var add1   = Fn.new {|x| x + 1}
var double = Fn.new {|x| x * 2}

System.print((double @< add1).call(1)) // expect: 4

// Check operator priority
System.print(1 |> double @< add1) // expect: 4
System.print(double @< add1 |< 1) // expect: 4

// Swallow a trailing newline.
System.print(double @<
    add1 |< 1) // expect: 4
