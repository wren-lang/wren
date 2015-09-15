var a = [1, 2, 3]

System.print(a.count {|x| x > 3 })      // expect: 0
System.print(a.count {|x| x > 1 })      // expect: 2

System.print([].count {|x| true })      // expect: 0
