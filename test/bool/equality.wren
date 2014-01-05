IO.print(true == true)    // expect: true
IO.print(true == false)   // expect: false
IO.print(false == true)   // expect: false
IO.print(false == false)  // expect: true

// Not equal to other types.
IO.print(true == 1)        // expect: false
IO.print(false == 0)       // expect: false
IO.print(true == "true")   // expect: false
IO.print(false == "false") // expect: false
IO.print(false == "")      // expect: false

IO.print(true != true)    // expect: false
IO.print(true != false)   // expect: true
IO.print(false != true)   // expect: true
IO.print(false != false)  // expect: false

// Not equal to other types.
IO.print(true != 1)        // expect: true
IO.print(false != 0)       // expect: true
IO.print(true != "true")   // expect: true
IO.print(false != "false") // expect: true
IO.print(false != "")      // expect: true
