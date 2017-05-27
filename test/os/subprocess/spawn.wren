import "os" for Subprocess

var callBack = Fn.new { |PID| System.print("PID: %( PID )")} // expect: PID: 1234

Subprocess.spawn(["file", "/tmp/foo.bar"], callBack) 
