class Foo {}

(new Foo).someUnknownMethod // expect runtime error: Foo does not implement 'someUnknownMethod'.
