^title File Class

Lets you work with files on the file system. An instance of this class
represents an open file with a file descriptor.

When you are done with a file object, it's a good idea to explicitly close it.
If you don't, the GC will close it when the file is no longer used and gets
finalized, but that may take a while. In the meantime, leaving it open wastes
a file descriptor.

## Static Methods

### File.**open**(path, fn)

Opens the file at [path] and passes it to [fn]. After the function returns, the
file is automatically closed.

    :::wren
    File.open("words.txt") {|file|
      file.readBytes(5)
    }

### File.**read**(path)

Reads the entire contents of the file at [path] and returns it as a string.

    :::wren
    File.read("words.txt")

The encoding or decoding is done. If the file is UTF-8, then the resulting
string will be a UTF-8 string. Otherwise, it will be a string of bytes in
whatever encoding the file uses.

### File.**size**(path)

Returns the size in bytes of the contents of the file at [path].

## Constructors

### File.**open**(path)

Opens the file at [path] for reading.

## Methods

### **descriptor**

The numeric file descriptor used to access the file.

### **isOpen**

Whether the file is still open or has been closed.

### **size**

The size of the contents of the file in bytes.

### **close**()

Closes the file. After calling this, you can read or write from it.

### **readBytes**(count)

Reads up to [count] bytes starting from the beginning of the file.

(Allowing an offset to read elsewhere from the file isn't implemented yet.)
