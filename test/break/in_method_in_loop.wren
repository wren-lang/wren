var done = false
while (!done) {
  class Foo {
    method {
      break // expect error
    }
  }
  done = true
}
