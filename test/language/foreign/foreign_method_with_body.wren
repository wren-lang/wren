class Foo {
  foreign def method { "body" } // expect error
} // expect error

// The second error is cascaded.
