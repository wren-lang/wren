^title Stat Class

A data structure describing the low-level details of a file system entry.

## Static Methods

### Stat.**path**(path)

"Stats" the file or directory at `path`.

## Methods

### **device**

The ID of the device containing the entry.

### **inode**

The [inode][] number of the entry.

[inode]: https://en.wikipedia.org/wiki/Inode

### **mode**

A bit field describing the entry's type and protection flags.

### **linkCount**

The number of hard links to the entry.

### **user**

Numeric user ID of the file's owner.

### **group**

Numeric group ID of the file's owner.

### **specialDevice**

The device ID for the entry, if it's a special file.

### **size**

The size of the entry in bytes.

### **blockSize**

The preferred block size in bytes for interacting with the file. It may vary
from file to file.

### **blockCount**

The number of system blocks allocated on disk for the file.
