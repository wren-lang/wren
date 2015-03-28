// Infinite iterator demonstrating that Sequence.map is not eager
class FibIterator {
  new {
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
  iterate(iterator) {
    if (iterator == null) return new FibIterator
    iterator.iterate
    return iterator
  }

  iteratorValue(iterator) { iterator.value }
}

var squareFib = (new Fib).map {|fib| fib * fib }
var iterator = null

IO.print(squareFib is Sequence) // expect: true
IO.print(squareFib) // expect: instance of MapSequence

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iteratorValue(iterator)) // expect: 0

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iteratorValue(iterator)) // expect: 1

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iteratorValue(iterator)) // expect: 1

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iteratorValue(iterator)) // expect: 4

iterator = squareFib.iterate(iterator)
IO.print(squareFib.iteratorValue(iterator)) // expect: 9
