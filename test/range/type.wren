var range = 2..5

IO.print(range is Range)      // expect: true
IO.print(range is Object)     // expect: true
IO.print(range is String)     // expect: false
IO.print(range.type == Range) // expect: true
