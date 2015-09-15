class TestSequence is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == null) return 1
    if (iterator == 10) return false
    return iterator + 1
  }

  iteratorValue(iterator) { iterator }
}

System.print(TestSequence.new().count) // expect: 10
