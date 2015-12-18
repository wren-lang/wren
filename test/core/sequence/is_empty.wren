System.print([].isEmpty) // expect: true
System.print([1].isEmpty) // expect: false

class InfiniteSequence is Sequence {
  def construct new() {}
  def iterate(iterator) { true }
  def iteratorValue(iterator) { iterator }
}

// Should not try to iterate the whole sequence.
System.print(InfiniteSequence.new().isEmpty) // expect: false
