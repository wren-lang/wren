import "os" for Subprocess

var cat = Subprocess.spawn(["curl", "https://api.cryptonator.com/api/ticker/btc-eth"])
var catOut = cat.stdOut

cat.onOutCB = Fn.new {
	var out = catOut.call()
	System.print("out: %( out )")
} 

/*doagain*/

/*cat = Subprocess.spawn(["ls", "/"])*/
/*catOut = cat.stdOut*/

/*out = catOut.call()*/
/*System.print("out: %( out )")*/

/*cat.onOutCB = Fn.new {*/
	/*var out = catOut.call()*/
	/*System.print("out: %( out )")*/
/*} */
