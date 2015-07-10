var done = false
while (!done) {
  Fn.new {
    break // expect error
  }
  done = true
}