io.write(true == true)    // expect: true
io.write(true == false)   // expect: false
io.write(false == true)   // expect: false
io.write(false == false)  // expect: true

// Not equal to other types.
io.write(true == 1)        // expect: false
io.write(false == 0)       // expect: false
io.write(true == "true")   // expect: false
io.write(false == "false") // expect: false
io.write(false == "")      // expect: false

io.write(true != true)    // expect: false
io.write(true != false)   // expect: true
io.write(false != true)   // expect: true
io.write(false != false)  // expect: false

// Not equal to other types.
io.write(true != 1)        // expect: true
io.write(false != 0)       // expect: true
io.write(true != "true")   // expect: true
io.write(false != "false") // expect: true
io.write(false != "")      // expect: true
