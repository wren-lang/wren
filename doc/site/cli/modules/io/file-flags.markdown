^title FileFlags Class

Contains constants for the various file flags used to open or create a file.
These correspond directly to the flags that can be passed to the POSIX
[`open()`][open] syscall.

[open]: http://linux.die.net/man/2/open

They are integers and can be bitwise or'ed together to produce a composite
flag.

## Static Methods

### FileFlags.**readOnly**

The file can be read from but not written. Equivalent to `O_RDONLY`.

### FileFlags.**writeOnly**

The file can be written but not read from. Equivalent to `O_WRONLY`.

### FileFlags.**readWrite**

The file can be both read from and written to. Equivalent to `O_RDWR`.

### FileFlags.**sync**

Writes will block until the data has been physically written to the underling
hardware. This does *not* affect whether or the file API is synchronous. File
operations are always asynchronous in Wren and may allow other scheduled fibers
to run.

This is a lower-level flag that ensures that when a write completes, it has
been flushed all the way to disc.

### FileFlags.**create**

Creates a new file if a file at the given path does not already exist.

### FileFlags.**truncate**

If the file already exists and can be written to, its previous contents are
discarded.

### FileFlags.**exclusive**

Ensures that a new file must be created. If a file already exists at the given
path, this flag will cause the operation to fail.
