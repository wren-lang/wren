var words = ""
["One", "Two", "Three"].each {|word| words = words + word }
IO.print(words) // expect: OneTwoThree
