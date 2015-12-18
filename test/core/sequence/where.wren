// Infinite iterator demonstrating that Sequence.where is not eager
class FibIterator {
  def construct new() {
    _current = 0
    _next = 1
  }

  def iterate {
    var sum = _current + _next
    _current = _next
    _next = sum
  }

  def value { _current }
}

class Fib is Sequence {
  def construct new() {}

  def iterate(iterator) {
    if (iterator == null) return FibIterator.new()
    iterator.iterate
    return iterator
  }

  def iteratorValue(iterator) { iterator.value }
}

var largeFibs = Fib.new().where {|fib| fib > 100 }
var iterator = null

System.print(largeFibs is Sequence) // expect: true
System.print(largeFibs) // expect: instance of WhereSequence

iterator = largeFibs.iterate(iterator)
System.print(largeFibs.iteratorValue(iterator)) // expect: 144

iterator = largeFibs.iterate(iterator)
System.print(largeFibs.iteratorValue(iterator)) // expect: 233

iterator = largeFibs.iterate(iterator)
System.print(largeFibs.iteratorValue(iterator)) // expect: 377
