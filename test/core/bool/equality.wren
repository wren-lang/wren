System.print(true == true)    // expect: true
System.print(true == false)   // expect: false
System.print(false == true)   // expect: false
System.print(false == false)  // expect: true

// Not equal to other types.
System.print(true == 1)        // expect: false
System.print(false == 0)       // expect: false
System.print(true == "true")   // expect: false
System.print(false == "false") // expect: false
System.print(false == "")      // expect: false

System.print(true != true)    // expect: false
System.print(true != false)   // expect: true
System.print(false != true)   // expect: true
System.print(false != false)  // expect: false

// Not equal to other types.
System.print(true != 1)        // expect: true
System.print(false != 0)       // expect: true
System.print(true != "true")   // expect: true
System.print(false != "false") // expect: true
System.print(false != "")      // expect: true
