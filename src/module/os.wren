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

	//loads more data into stdin
	stdIn=(x){
		__stdInBuffers[_pid] = stdInBuffer + x
	}

	//gets a fiber that will yield everything from stdout since it last yielded
	//WARNING: calling one fiber will clear the global buffer for that PID,
	//avoid getting multipul stdOut's
	stdOut { Fiber.new {
		while(!_exitCode){
			var stdOut = __stdOutBuffers[_pid]
			__stdOutBuffers[_pid] = ""
			Fiber.yield(stdOut)
		}
	} }

	//register a callback to be called when we have more data from stdOut
	onOutCB=(cb){
		__onOutCBs[_pid] = cb
	}

	//register a callback to be called when the process exits
	onExitCB=(cb){
		__onExitCBs[_pid] = cb
	}

	//will launch a subproces, and calls fn with the process' PID
	construct spawn(command){
		//create the static buffers if they do not exist
		if(!__stdInBuffers){
			__stdInBuffers = {}
		}

		if(!__stdOutBuffers){
			__stdOutBuffers = {}
		}

		if(!__onOutCBs){
			__onOutCBs = {}
		}

		if(!__onExitCBs){
			__onExitCBs = {}
		}

		_pid = Subprocess.spawn_(command)

		__stdInBuffers[_pid] = ""
		__stdOutBuffers[_pid] = ""

		__onOutCBs[_pid] = Fn.new {}
		__onExitCBs[_pid] = Fn.new {}
	}

	static recieveStdOut_(pid, stdOut){
		__stdOutBuffers[pid] = __stdOutBuffers[pid] + stdOut
		__onOutCBs[pid].call()
	}

	foreign static spawn_(command)
}
