class Test {
  construct new() {}
  ~~(needle) { 42 }
}

var test = Test.new()

System.print(1 ~~ test) // expect: 42
