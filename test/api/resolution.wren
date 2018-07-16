class Resolution {
  foreign static noResolver()
  foreign static returnsNull()
  foreign static changesString()
  foreign static shared()
  foreign static importer()
}

// If no resolver function is configured, the default resolver just passes
// along the import string unchanged.
System.print(Resolution.noResolver())
// expect: loading foo/bar
// expect: ok
// expect: success

// If the resolver returns NULL, it's reported as an error.
System.print(Resolution.returnsNull())
// expect: Could not resolve module 'foo/bar' imported from 'main'.
// expect: error

// The resolver function can change the string.
System.print(Resolution.changesString())
// expect: loading main/foo/bar
// expect: ok
// expect: success

// Imports both "foo/bar" and "foo|bar", but only loads the module once because
// they resolve to the same module.
System.print(Resolution.shared())
// expect: loading main/foo/bar
// expect: ok
// expect: success

// The string passed as importer is the resolver string of the importing module.
System.print(Resolution.importer())
// expect: loading main/baz/bang
// expect: loading main/baz/bang/foo/bar
// expect: ok
// expect: success
