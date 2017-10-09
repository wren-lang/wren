^title Stdin Class

The standard input stream.

## Static Methods

### **isRaw**

Returns `true` if stdin is in raw mode. When in raw mode, input is not echoed
or buffered, and all characters, even non-printing and control characters go
into stdin.

Defaults to `false`.

### **isRaw**=(value)

Sets raw mode on or off.

### **isTerminal**

Returns `true` if Stdin is connected to a "TTY". This is true when the user is
running Wren in an interactive terminal, and false if it its input is coming
from a pipe.

### **readByte**()

Reads one byte of input from stdin. Blocks the current fiber until a byte has
been received.

Returns the byte value as a number or `null` if stdin is closed.

Note that output is not automatically flushed when calling this. If you want to
display a prompt before reading input, you'll want to call `Stdout.flush()`
after printing the prompt.

### **readLine**()

Reads one line of input from stdin. Blocks the current fiber until a full line
of input has been received.

Returns the string of input or `null` if stdin is closed.

Note that output is not automatically flushed when calling this. If you want to
display a prompt before reading input, you'll want to call `Stdout.flush()`
after printing the prompt.
