var done = false
while (!done) {
  Fn.new {
    continue // expect error
  }
  done = true
}