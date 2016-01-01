^title Stdin Class

The standard input stream.

## Static Methods

### **readLine**()

Reads one line of input from stdin. Blocks the current fiber until a full line
of input has been received.

Returns the string of input or `null` if stdin is closed.
