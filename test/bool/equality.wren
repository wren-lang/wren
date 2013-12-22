IO.write(true == true)    // expect: true
IO.write(true == false)   // expect: false
IO.write(false == true)   // expect: false
IO.write(false == false)  // expect: true

// Not equal to other types.
IO.write(true == 1)        // expect: false
IO.write(false == 0)       // expect: false
IO.write(true == "true")   // expect: false
IO.write(false == "false") // expect: false
IO.write(false == "")      // expect: false

IO.write(true != true)    // expect: false
IO.write(true != false)   // expect: true
IO.write(false != true)   // expect: true
IO.write(false != false)  // expect: false

// Not equal to other types.
IO.write(true != 1)        // expect: true
IO.write(false != 0)       // expect: true
IO.write(true != "true")   // expect: true
IO.write(false != "false") // expect: true
IO.write(false != "")      // expect: true
