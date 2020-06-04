^title Directory Class

A directory on the file system.

## Static Methods

### Directory.**exists**(path)

Whether a directory exists at `path`. This returns `false` for files or other
special file system entities.

### Directory.**list**(path)

Lists the contents of the directory at `path`. Returns a sorted list of path
strings for all of the contents of the directory.
