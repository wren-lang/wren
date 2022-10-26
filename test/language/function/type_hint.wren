System.print(Fn.new {                                  -> String ""                   }.call())              // expect: 
System.print(Fn.new {||                                -> String "||"                 }.call())              // expect: ||
System.print(Fn.new {| |                               -> String "| |"                }.call())              // expect: | |
System.print(Fn.new {|a: String|                       -> String "|%(a)|"             }.call("a"))           // expect: |a|
System.print(Fn.new {|a: String, b: String|            -> String "|%(a), %(b)|"       }.call("a", "b"))      // expect: |a, b|
System.print(Fn.new {|a: String, b: String, c: String| -> String "|%(a), %(b), %(c)|" }.call("a", "b", "c")) // expect: |a, b, c|
