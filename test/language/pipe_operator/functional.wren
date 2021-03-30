class Pipe {
    construct new(value) {
        _value = value
    }

    |> (other) {
        return other.call(_value)
    }

    toString {_value}
}

var Concat = Fn.new {|message| Pipe.new(message + "World")}
var Multiply = Fn.new {|message, count| Pipe.new(message * count)}
var MultiplyCurried = Fn.new {|value| Multiply.call(value.toString, 2)}
var Print = Fn.new {|message| System.print(message)}

// It can help making functional composition easier
Pipe.new("Hello") |> Concat |> MultiplyCurried |> Print // expect: HelloWorldHelloWorld