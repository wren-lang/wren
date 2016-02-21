^title Stat Class

A data structure describing the low-level details of a file system entry.

## Static Methods

### Stat.**path**(path)

"Stats" the file or directory at `path`.

## Methods

### **blockCount**

The number of system blocks allocated on disk for the file.

### **blockSize**

The preferred block size in bytes for interacting with the file. It may vary
from file to file.

### **device**

The ID of the device containing the entry.

### **group**

Numeric group ID of the file's owner.

### **inode**

The [inode][] number of the entry.

[inode]: https://en.wikipedia.org/wiki/Inode

### **isDirectory**

Whether the file system entity is a directory.

### **isFile**

Whether the file system entity is a regular file, as opposed to a directory or
other special entity.

### **linkCount**

The number of hard links to the entry.

### **mode**

A bit field describing the entry's type and protection flags.

### **size**

The size of the entry in bytes.

### **specialDevice**

The device ID for the entry, if it's a special file.

### **user**

Numeric user ID of the file's owner.
