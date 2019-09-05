// Regression test for https://github.com/munificent/wren/issues/510.
//
// Tests that re-entrant API calls are handled correctly. The host uses
// `wrenCall()` to invoke `CallCallsForeign.call()`. That in turn calls
// `CallCallsForeign.api()`, which goes back through the API.
class CallCallsForeign {
  foreign static api()

  static call(param) {
    System.print(api())
    // expect: slots before 2
    // expect: [1, 2, 3, 4, 5, 6, 7, 8, 9]
    System.print(param) // expect: parameter
    // expect: slots after 1
    return "result"
  }
}
