import "os" for Subprocess

var callBack = Fn.new { |status| System.print("status: %( status )")} // expect: status: 'STDOUT of [ls, /tmp/]'

Subprocess.call(["ls", "/tmp/"], callBack) 

