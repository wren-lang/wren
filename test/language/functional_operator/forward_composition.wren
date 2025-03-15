var add1   = Fn.new {|x| x + 1}
var double = Fn.new {|x| x * 2}

System.print((double @> add1).call(1)) // expect: 3

// Check operator priority
System.print(1 |> double @> add1) // expect: 3
System.print(double @> add1 |< 1) // expect: 3

// Swallow a trailing newline.
System.print(double @>
    add1 |< 1) // expect: 3
