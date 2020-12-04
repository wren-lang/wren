var done = false
while (!done) {
  class Foo {
    method {
      continue // expect error
    }
  }
  done = true
}
