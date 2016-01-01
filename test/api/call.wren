class Call {
  static def noParams {
    System.print("noParams")
  }

  static def zero() {
    System.print("zero")
  }

  static def one(one) {
    System.print("one %(one)")
  }

  static def two(one, two) {
    // Don't print null bytes.
    if (two is String && two.bytes.contains(0)) {
      two = two.bytes.toList
    }

    System.print("two %(one) %(two)")
  }

  static def getValue() { ["a", "b"] }
}

// expect: noParams
// expect: zero
// expect: one 1
// expect: two 1 2

// expect: two true false
// expect: two 1.2 3.4
// expect: two string another
// expect: two null [a, b]
// expect: two str [98, 0, 121, 0, 116, 0, 101]
// expect: one 0.1
