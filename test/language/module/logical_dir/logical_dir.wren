// Import a module from within a named package.
import "foo/within_foo" for Foo
// expect: ran foo module
// expect: ran bar module

System.print(Foo) // expect: from foo
