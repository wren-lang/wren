class Foo {
  System.print("one")

  construct new() {
    System.print("constructor")
  }

  System.print("two")

  construct other() {
    System.print("other constructor")
  }
}

// Does not execute body until object is constructed.
System.print("before") // expect: before

// Executes body before constructor body.
Foo.new()   // expect: one
            // expect: two
            // expect: constructor

// Executes body for each constructor.
Foo.other() // expect: one
            // expect: two
            // expect: other constructor

// TODO: Calling (and not calling) superclass constructor in subclass.
