class TestSequence is Sequence {
  iterate(iterator) {
    if (iterator == null) return 1
    if (iterator == 10) return false
    return iterator + 1
  }

  iteratorValue(iterator) { iterator }
}

IO.print((new TestSequence).count) // expect: 10
