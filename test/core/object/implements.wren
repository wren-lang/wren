class Foo {
  construct new() {}
}

class EmptyInterface {
}

class ConstructorInterface {
  construct constructor_should_be_ingored_by_implements() { subclassResponsibility() }
}

class IterableInterface {
  iterate(iterator)       { subclassResponsibility() }
  iteratorValue(iterator) { subclassResponsibility() }
}

class StaticMethodInterface {
  static static_methods_should_be_ingored_by_implements() { subclassResponsibility() }
}

// Everything implements the empty interface.
System.print(Object    implements EmptyInterface) // expect: true
System.print(Foo.new() implements EmptyInterface) // expect: true
System.print([]        implements EmptyInterface) // expect: true
System.print({}        implements EmptyInterface) // expect: true
System.print(0..1      implements EmptyInterface) // expect: true

// Everything should ignore constructors
System.print(Object    implements ConstructorInterface) // expect: true
System.print(Foo.new() implements ConstructorInterface) // expect: true
System.print([]        implements ConstructorInterface) // expect: true
System.print({}        implements ConstructorInterface) // expect: true
System.print(0..1      implements ConstructorInterface) // expect: true

// Test some plausible interfaces
System.print(Foo.new() implements IterableInterface) // expect: false
System.print([]        implements IterableInterface) // expect: true
System.print({}        implements IterableInterface) // expect: true
System.print(0..1      implements IterableInterface) // expect: true

// Everything should ignore static methods
System.print(Object    implements StaticMethodInterface) // expect: true
System.print(Foo.new() implements StaticMethodInterface) // expect: true
System.print([]        implements StaticMethodInterface) // expect: true
System.print({}        implements StaticMethodInterface) // expect: true
System.print(0..1      implements StaticMethodInterface) // expect: true

// Ignore newline after "implements".
System.print(123 implements
  Num) // expect: true
