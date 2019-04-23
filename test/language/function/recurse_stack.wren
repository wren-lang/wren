var count = 0
var f

f = Fn.new {|n|
  if (n == 0) count = count + 1 else f.call(n - 1) 
}

f.call(10)
System.print(count) // expect: 1
