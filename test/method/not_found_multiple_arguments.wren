class Foo {}

(new Foo).someUnknownMethod(1, 2) // expect runtime error: Foo does not implement method 'someUnknownMethod' with 2 arguments.