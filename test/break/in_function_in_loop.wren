var done = false
while (!done) {
  fn {
    break // expect error
  }
  done = true
}