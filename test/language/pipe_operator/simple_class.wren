class Test {
    construct new(value) {
        _value = value
    }

    |> (other) {
        System.print(_value + other)
    }
}

Test.new("Hello") |> "World" // expect: HelloWorld