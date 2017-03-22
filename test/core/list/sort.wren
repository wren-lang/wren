class A {
    value { _value }

    toString { _value.toString }

    construct new(value) {
        _value = value
    }

    <=(other) { _value <= other.value }
}

System.print([3, 1, 5, 9, 7].sort()) // expect: [1, 3, 5, 7, 9]

System.print([A.new(3), A.new(1), A.new(5), A.new(9), A.new(7)].sort()) // expect: [1, 3, 5, 7, 9]

System.print([A.new(3), A.new(1), A.new(5), A.new(9), A.new(7)].sort{|n| -n.value }) // expect: [9, 7, 5, 3, 1]
