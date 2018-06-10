class Platform {
  foreign static isPosix
  foreign static name

  static isWindows { name == "Windows" }
}

class Process {
  // TODO: This will need to be smarter when wren supports CLI options.
  static arguments { allArguments[2..-1] }
  foreign static environ_

  static environ {
    var envMap = {}

    for (i in environ_) {
      var r = i.split("=")
      envMap[r[0]] = r[1]
    }

    return envMap
  }

  foreign static allArguments
}
