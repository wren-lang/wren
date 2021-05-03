// Fiber
var fi1 = Fiber.new {}
var fi2 = Fiber.new {}
System.print(fi1 ~~ fi1) // expect: true
System.print(fi2 ~~ fi2) // expect: true
System.print(fi1 ~~ fi2) // expect: false
System.print(fi2 ~~ fi1) // expect: false

// Fn
var fn1 = Fn.new { 42 }
var fn2 = Fn.new { |x| x+1 }
System.print(fn1 ~~ fn1)      // expect: 42
System.print(1 ~~ fn1)        // expect: 42
System.print(null ~~ fn1)     // expect: 42
System.print("str" ~~ fn1)    // expect: 42
System.print(Num ~~ fn1)      // expect: 42

System.print(1 ~~ fn2)        // expect: 2
System.print(-1 ~~ fn2)       // expect: 0
var strfiber = Fiber.new { "x" ~~ fn2 }
var error = strfiber.try()
System.print(error) // expect: Right operand must be a string.

// System
System.print(System ~~ System)  // expect: true
System.print(true ~~ System)    // expect: false
System.print(1 ~~ System)       // expect: false
System.print("a" ~~ System)     // expect: false
System.print(fi1 ~~ System)     // expect: false
System.print(fn1 ~~ System)     // expect: false

// Note: not testing Meta or Random specifically, since they might not be
// compiled into Wren.

// Object
class Test {
  construct new() {}
  // inherits Object.~~(_)
}
var t1 = Test.new()
var t2 = Test.new()

System.print(t1 ~~ t1) // expect: true
System.print(t2 ~~ t2) // expect: true
System.print(t1 ~~ t2) // expect: false
System.print(t2 ~~ t1) // expect: false

class TestChild is Test {
  construct new() {}
}
var tc = TestChild.new()

// Class: values against classes
System.print(t1 ~~ Test)  // expect: true
System.print(t1 ~~ Num)   // expect: false

System.print(t1 ~~ TestChild) // expect: false
System.print(tc ~~ TestChild) // expect: true
System.print(tc ~~ Test)      // expect: true

System.print(fi1 ~~ Fiber)        // expect: true
System.print(fn1 ~~ Fn)           // expect: true
System.print(fn1 ~~ Fiber)        // expect: false
System.print(fi1 ~~ Fn)           // expect: false
System.print("a" ~~ String)       // expect: true
System.print("a" ~~ Num)          // expect: false
System.print(1 ~~ Num)            // expect: true
System.print(Num.nan ~~ Num)      // expect: true
System.print(Num.infinity ~~ Num) // expect: true

// Class: classes against classes
System.print(Test ~~ Test)        // expect: true
System.print(Test ~~ Object)      // expect: true
System.print(TestChild ~~ Test)   // expect: true
System.print(TestChild ~~ Object) // expect: true
System.print(Test ~~ TestChild)   // expect: false
System.print(Test ~~ String)      // expect: false
System.print(String ~~ Test)      // expect: false
