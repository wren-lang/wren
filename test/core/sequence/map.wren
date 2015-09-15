// Infinite iterator demonstrating that Sequence.map is not eager
class FibIterator {
  construct new() {
    _current = 0
    _next = 1
  }

  iterate {
    var sum = _current + _next
    _current = _next
    _next = sum
  }

  value { _current }
}

class Fib is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == null) return FibIterator.new()
    iterator.iterate
    return iterator
  }

  iteratorValue(iterator) { iterator.value }
}

var squareFib = Fib.new().map {|fib| fib * fib }
var iterator = null

System.print(squareFib is Sequence) // expect: true
System.print(squareFib) // expect: instance of MapSequence

iterator = squareFib.iterate(iterator)
System.print(squareFib.iteratorValue(iterator)) // expect: 0

iterator = squareFib.iterate(iterator)
System.print(squareFib.iteratorValue(iterator)) // expect: 1

iterator = squareFib.iterate(iterator)
System.print(squareFib.iteratorValue(iterator)) // expect: 1

iterator = squareFib.iterate(iterator)
System.print(squareFib.iteratorValue(iterator)) // expect: 4

iterator = squareFib.iterate(iterator)
System.print(squareFib.iteratorValue(iterator)) // expect: 9
