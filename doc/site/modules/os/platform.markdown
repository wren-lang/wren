^title Platform Class

The Platform class exposes basic information about the operating system Wren is
running on top of.

## Static Methods

### **name**

The name of the platform. This roughly describes the operating system, and is
usually one of:

* "iOS"
* "Linux"
* "OS X"
* "POSIX"
* "Unix"
* "Windows"

If Wren was compiled for an unknown operating system, returns "Unknown".

### **isPosix**

Returns `true` if the host operating system is known to support the POSIX
standard. This is true for Linux and other Unices, as well as the various Apple
operating systems.

### **isWindows**

Returns `true` if the host operating system is some flavor of Windows.
