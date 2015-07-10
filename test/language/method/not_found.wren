class Foo {}

Foo.new().someUnknownMethod // expect runtime error: Foo does not implement 'someUnknownMethod'.
