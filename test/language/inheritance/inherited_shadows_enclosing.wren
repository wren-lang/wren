class Base {
  def method() { System.print("Base") }
}

class Outer {
  construct new() {}

  def method() { System.print("Outer") }

  class Inner is Base {
    construct new() {}
  }
}

Outer.new().Inner.new().method() // expect: Base
