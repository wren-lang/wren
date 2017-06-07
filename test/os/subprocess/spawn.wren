import "os" for Subprocess

var subprocess = Subprocess.spawn(["cat", "/tmp/foo.bar"])

System.print(subprocess.stdout) //expect big bants
