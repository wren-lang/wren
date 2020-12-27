// Get the first item which pass the condition
System.print([1, 2, 3].get {|item| item > 1}) // expect: 2

System.print([1, 2, 3].get {|item| item > 3}) // expect: null