class TestSequence is Sequence {
  construct new() {}

  iterate(iterator) {
    if (iterator == null) return 1
    if (iterator == 3) return false
    return iterator + 1
  }

  iteratorValue(iterator) { iterator }
}

var test = TestSequence.new().take(3)

System.print(test is Sequence) // expect: true
System.print(test) // expect: instance of TakeSequence

// Taking 0 produces empty list.
System.print(test.take(0).isEmpty) // expect: true

// Taking 1 works.
System.print(test.take(1).toList) // expect: [1]

// Taking more than length of sequence produces whole sequence.
System.print(test.take(4).toList) // expect: [1, 2, 3]
