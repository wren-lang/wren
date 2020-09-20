class Test {
  construct new() {}
  test0() {
    System.print("test0")
    return this
  }
  test1() {
    System.print("test1")
    return this
  }
  test2() {
    System.print("test2")
    return this
  }

  [index] {
    System.print("testSubscript")
    return this
  }
}

class Tester {
  construct new() {

    var test = _test = Test.new()

    //test local access

    test.
      test0().          // expect: test0
      test1().          // expect: test1
      test2()           // expect: test2

    test
      .test0()          // expect: test0
      .test1()          // expect: test1
      .test2()          // expect: test2

    test
      .test0()          // expect: test0
      .test1().         // expect: test1
       test2()          // expect: test2

    test[0]             // expect: testSubscript
      .test0()          // expect: test0

    test[0].            // expect: testSubscript
      test0()           // expect: test0

    //test field access

    _test.
      test0().          // expect: test0
      test1().          // expect: test1
      test2()           // expect: test2

    _test
      .test0()          // expect: test0
      .test1()          // expect: test1
      .test2()          // expect: test2

    _test
      .test0().         // expect: test0
       test1().         // expect: test1
       test2()          // expect: test2

    _test[0]            // expect: testSubscript
      .test0()          // expect: test0

    _test[0].           // expect: testSubscript
      test0()           // expect: test0

  }

  getter { _test }
  method() { _test }

} //Tester

//access via methods/getter

var external = Tester.new()

external.getter.
  test0().              // expect: test0
  test1().              // expect: test1
  test2()               // expect: test2

external.getter
  .test0()              // expect: test0
  .test1()              // expect: test1
  .test2()              // expect: test2

external.getter.
   test0()              // expect: test0
  .test1()              // expect: test1
  .test2()              // expect: test2

external.getter[0].     // expect: testSubscript
   test0()              // expect: test0

external.getter[0]      // expect: testSubscript
   .test0()             // expect: test0

external.method().
  test0().              // expect: test0
  test1().              // expect: test1
  test2()               // expect: test2

external.method()
  .test0()              // expect: test0
  .test1()              // expect: test1
  .test2()              // expect: test2

external.method().
  test0()               // expect: test0
  .test1().             // expect: test1
  test2()               // expect: test2

external.method()[0].   // expect: testSubscript
  test0()               // expect: test0

external.method()[0]    // expect: testSubscript
  .test0()              // expect: test0


//regular access in module scope

var other = Test.new()

other.
  test0().              // expect: test0
  test1().              // expect: test1
  test2()               // expect: test2

other
  .test0()              // expect: test0
  .test1()              // expect: test1
  .test2()              // expect: test2

other
  .test0().             // expect: test0
   test1()              // expect: test1
  .test2()              // expect: test2


other[0]                // expect: testSubscript
  .test0()              // expect: test0

other[0].               // expect: testSubscript
  test0()               // expect: test0
