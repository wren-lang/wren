// Refering to an instance method in a nested static class should *not* walk
// out to find the nearest enclosing instance method. We could make that work,
// but it's confusing to users, and would require some tricky work to make sure
// the enclosing instance is closed over.

class Outer {
  foo {
    class Inner {
      static bar {
        _field = "nope" // expect error
      }
    }
  }
}
