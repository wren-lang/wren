import "os" for Subprocess

var cat = Subprocess.spawn(["ls", "/"])
var catOut = cat.stdOut

var out = catOut.call()
System.print("out: %( out )")

cat.onOutCB = Fn.new {
	var out = catOut.call()
	System.print("out: %( out )")
} 
