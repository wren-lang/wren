class BytecodeLoader {
  foreign static runTests()
}

var result = BytecodeLoader.runTests()
// expect: true
System.print(result)
