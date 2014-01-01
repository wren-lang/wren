class Foo {}

(new Foo).someUnknownMethod // expect runtime error: Receiver does not implement method 'someUnknownMethod'.
