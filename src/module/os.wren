class Platform {
  foreign static isPosix
  foreign static name

  static isWindows { name == "Windows" }
}

class Process {
  // TODO: This will need to be smarter when wren supports CLI options.
  static arguments { allArguments[2..-1] }

  foreign static allArguments
}

class Subprocess {
	pid { _pid }
	exitCode { _exitCode }
	stdout { _stdout }

	//will launch a subproces, and calls fn with the process' PID
	construct spawn(command){
		_stdout = Subprocess.spawn_(command)
		_pid = null
		_exitCode = null
	}

	foreign static spawn_(command)
}
