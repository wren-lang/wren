var done = false
while (!done) {
  class Foo {
    def method {
      break // expect error
    }
  }
  done = true
}
