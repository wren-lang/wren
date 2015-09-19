class EmptyMethodName {
  construct new() {}
  () {
    System.print("Hello World")
  }

  (a) {
    System.print(a)
  }
}

var fn = EmptyMethodName.new()

fn.() // expect: Hello World
fn.("foo") // expect: foo
