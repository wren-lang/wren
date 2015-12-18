class TestSequence is Sequence {
  construct new() {}

  def iterate(iterator) {
    if (iterator == null) return 1
    if (iterator == 10) return false
    return iterator + 1
  }

  def iteratorValue(iterator) { iterator }
}

System.print(TestSequence.new().count) // expect: 10
