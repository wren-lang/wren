class TestSequence is Sequence {
  iterate(iterator) {
    if (iterator == null) return 1
    if (iterator == 3) return false
    return iterator + 1
  }

  iteratorValue(iterator) { iterator }
}

IO.print((new TestSequence).list) // expect: [1, 2, 3]
