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
	//will launch a subproces, and calls fn with the process' PID
	static spawn(command, fn){
		fn.call(spawn_(command, fn))
	}
	//will launch a subprocess, and run a function with the subprocess' STDOUT
	//once the subprocess has terminated
	static call(command, fn){

		fn.call("'STDOUT of %(command)'")
	}

	foreign static spawn_(command, fn)
	foreign static call_(command, fn)
}
