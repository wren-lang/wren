Fn.new {
  System.print(notDefined)
}.call()
// Error on last line because it assumes the variable gets declared later at
// the module level.
// expect error