System.print(Fn.new {    ""    }.call()) // expect: 
System.print(Fn.new {||  "||"  }.call()) // expect: ||
System.print(Fn.new {| | "| |" }.call()) // expect: | |
