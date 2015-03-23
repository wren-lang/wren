var a = [1, 2, 3]

IO.print(a.count {|x| x > 3 })      // expect: 0
IO.print(a.count {|x| x > 1 })      // expect: 2

IO.print([].count {|x| true })      // expect: 0
