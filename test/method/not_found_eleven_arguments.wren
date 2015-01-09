class Foo {}

(new Foo).someUnknownMethod(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) // expect runtime error: Foo does not implement method 'someUnknownMethod' with 11 arguments.