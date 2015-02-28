class Foo {}

(new Foo).someUnknownMethod(1) // expect runtime error: Foo does not implement 'someUnknownMethod(_)'.