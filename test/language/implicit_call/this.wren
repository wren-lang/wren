class CallArg {
  static test() {
    this("string") // expect: string
  }

  static call(arg) { System.print(arg) }
}

class CallBlock {
  static test() {
    this { "block" } // expect: block
  }

  static call(block) { System.print(block()) }
}

class CallBoth {
  static test() {
    this(1, 2) { 3 } // expect: 1 2 3
  }

  static call(a, b, c) { System.print("%(a) %(b) %(c())") }
}

CallArg.test()
CallBlock.test()
CallBoth.test()
