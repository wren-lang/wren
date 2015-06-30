IO.print([].isEmpty) // expect: true
IO.print([1].isEmpty) // expect: false

class InfiniteSequence is Sequence {
  iterate(iterator) { true }
  iteratorValue(iterator) { iterator }
}

// Should not try to iterate the whole sequence.
IO.print((new InfiniteSequence).isEmpty) // expect: false
