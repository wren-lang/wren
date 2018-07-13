^title File Class

Lets you work with files on the file system. An instance of this class
represents an open file with a file descriptor.

When you are done with a file object, it's a good idea to explicitly close it.
If you don't, the GC will close it when the file is no longer used and gets
finalized, but that may take a while. In the meantime, leaving it open wastes
a file descriptor.

## Static Methods

### File.**create**(path, fn)

Opens the file at `path` for writing and passes it to `fn`. If there is already
a file at that path, it is truncated. After the function returns, the file is
automatically closed.

    :::wren
    File.create("numbers.txt") {|file|
      file.writeBytes("one two three")
    }

### File.**delete**(path)

Deletes the file at `path`.

### File.**exists**(path)

Whether a regular file exists at `path`. This returns `false` for directories
or other special file system entities.

### File.**open**(path, fn)

Opens the file at `path` for reading and passes it to `fn`. After the function
returns, the file is automatically closed.

    :::wren
    File.open("words.txt") {|file|
      file.readBytes(5)
    }

### File.**read**(path)

Reads the entire contents of the file at `path` and returns it as a string.

    :::wren
    File.read("words.txt")

No encoding or decoding is done. If the file is UTF-8, then the resulting
string will be a UTF-8 string. Otherwise, it will be a string of bytes in
whatever encoding the file uses.

### File.**realPath**(path)

Resolves `path`, traversing symlinks and removining any unneeded `./` and `../`
components. Returns the canonical absolute path to the file.

    :::wren
    var path = "/some/./symlink/a/../b/file.txt"
    System.print(File.realPath(path)) //> /real/path/a/file.txt

### File.**size**(path)

Returns the size in bytes of the contents of the file at `path`.

## Constructors

### File.**create**(path)

Opens the file at `path` for writing. If there is already a file at that path,
it is truncated.

    :::wren
    var file = File.create("colors.txt")
    file.writeBytes("chartreuse lime teal")
    file.close()

### File.**open**(path)

Opens the file at `path` for reading. You are responsible for closing it when
done with it.

## Methods

### **descriptor**

The numeric file descriptor used to access the file.

### **isOpen**

Whether the file is still open or has been closed.

### **size**

The size of the contents of the file in bytes.

### **close**()

Closes the file. After calling this, you can't read or write from it.

### **readBytes**(count)

Reads up to `count` bytes starting from the beginning of the file.

    :::wren
    // Assume this file contains "I am a file!".
    File.open("example.txt") {|file|
      System.print(file.readBytes(6)) //> I am a
    }

### **readBytes**(count, offset)

Reads up to `count` bytes starting at `offset` bytes from the beginning of
the file.

    :::wren
    // Assume this file contains "I am a file!".
    File.open("example.txt") {|file|
      System.print(file.readBytes(6, 2)) //> am a f
    }

### **writeBytes**(bytes)

Writes the raw bytes of the string `bytes` to the end of the file.

### **writeBytes**(bytes, offset)

Writes the raw bytes of the string `bytes` to the to the file, starting at
`offset`. Any overlapping bytes already in the file at the offset are
overwritten.
