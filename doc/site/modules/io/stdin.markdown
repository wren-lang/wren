^title Stdin Class

The standard input stream.

## Static Methods

### **readByte**()

Reads one byte of input from stdin. Blocks the current fiber until a byte has
been received.

Returns the byte value as a number or `null` if stdin is closed.

### **readLine**()

Reads one line of input from stdin. Blocks the current fiber until a full line
of input has been received.

Returns the string of input or `null` if stdin is closed.
