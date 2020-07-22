foreign class Foo {
    construct unknownConstructor() {} // expect runtime error: Foo metaclass does not implement 'init unknownConstructor()'.
}

Foo.unknownConstructor()