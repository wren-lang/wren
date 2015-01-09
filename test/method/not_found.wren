class Foo {}

(new Foo).someUnknownMethod // expect runtime error: Foo does not implement method 'someUnknownMethod' with 0 arguments.
