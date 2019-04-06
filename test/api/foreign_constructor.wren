// Class with a special constructor.
foreign class Adder {
  construct new(values) {}
  foreign total
}

var adder = Adder.new([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])
System.print(adder.total) // expect: 45
