import "os" for Platform

// It's just a less stringly-typed API for checking the name.
System.print(Platform.isWindows == (Platform.name == "Windows")) // expect: true
