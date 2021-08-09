String.trait("answer", 42)
String.trait("world", Fn.new{|self| "%(self) World"})

System.print("Hello".trait("world")) // expect: Hello World
System.print(String.trait("answer")) // expect: 42

class NumExt {
    construct new(value) {
        _value = value
    }

    triple {
        return _value * 3
    }
}

Num.use(NumExt)

System.print(12.trait(NumExt).triple) // expect: 36