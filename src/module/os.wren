class Platform {
  foreign static isPosix
  foreign static name
}

class Process {
  // TODO: This will need to be smarter when wren supports CLI options.
  static arguments { allArguments[2..-1] }

  foreign static allArguments
}
