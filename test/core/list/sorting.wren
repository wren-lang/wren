class A {
    value { _value }

    toString { _value.toString }

    construct new(value) {
        _value = value
    }

    <(other) { _value < other.value }
}

System.print([3, 1, 5, 9, 7].sorted()) // expect: [1, 3, 5, 7, 9]
System.print([1, 2, 1, 1, 2].sorted()) // expect: [1, 1, 1, 2, 2]
System.print([3, 1, 5, 9, 7].sorted {|n| -n }) // expect: [9, 7, 5, 3, 1]
System.print([8, 2, 3, 9].sorted {|n| -n }) // expect: [9, 8, 3, 2]

var l = [3, 1, 5, 9, 7]
l.sort()
System.print(l) // expect: [1, 3, 5, 7, 9]

l = [A.new(3), A.new(1), A.new(5), A.new(9), A.new(7)]
l.sort()
System.print(l) // expect: [1, 3, 5, 7, 9]

l.sort{|n| -n.value }
System.print(l) // expect: [9, 7, 5, 3, 1]
