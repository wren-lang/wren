var done = false
while (!done) {
  new Fn {
    break // expect error
  }
  done = true
}