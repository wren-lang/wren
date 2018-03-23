import "os" for Subprocess

var cat = Subprocess.spawn(["echo", "foo bar baz",])
var catOut = cat.stdOut

cat.onOutCB = Fn.new {
	var out = catOut.call()
	System.print("[%( out )]") // expect: [foo bar baz
	// expect: ]
} 

cat.onExitCB = Fn.new { |exitCode|
	System.print("e[%( exitCode )]") // expect: e[0]
}
