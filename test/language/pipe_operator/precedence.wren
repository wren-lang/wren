class Test {
    construct new(value) {
        _value = value
    }

    |> (other) {
        return _value + other
    }
}

// Pipe Operator is just above Assignment precedence
var result = Test.new("Hello") |> "World" + "1"

System.print(result) // expect: HelloWorld1