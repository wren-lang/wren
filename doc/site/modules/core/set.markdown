^title Set Class

An orderered self-adjusting binary search tree variant called [Splay tree](https://en.wikipedia.org/wiki/Splay_tree) representing a [Set](https://en.wikipedia.org/wiki/Set_(mathematics)).

Allows fast Dynamic Set operations such as find, delete, min, max, successor and predecessor.

**Time complexity**: **Amortized O(log n)** in the worst case.

**Space complexity**: **O(n)** linear on the number of elements in the set.

## Constructors

### Set.**new()**

Creates a new initially empty set.

##### Complexity
**O(1)** time and space.

## Static Methods

### **Set.init()**

Called on the initialization of the Set class to define the `SetNode` class.

##### Note
There is not point of calling this function again.

## Methods

### **insert(item)**

Inserts a new item into the set.

##### Note
If the item can be equated via the `==` operator the the insertion is discarded. The newly inserted item is moved to the root of the tree
allowing for constant time re-access.

##### Return value
Returns true if the insertion is successful, otherwise false.

##### Complexity
**Amortized O(log n)** time and **O(1)** space.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.insert(666)) //true
    System.print(set.insert(666)) //false, 666 is already in the set

### **remove(item)**

Removes an existing item from the set.

##### Note
Deletion is discarded if the item doesn't exist.

##### Return value
Returns true if the deletion is successful, otherwise false.

##### Complexity
**Amortized O(log n)** time.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.remove(0)) //false, because 0 doesn't exist in the set
    System.print(set.insert(666)) //true
    System.print(set.remove(666)) //true, 666 does exist in the set

### **contains(item)**

Checks if item is in the set.

##### Note
if item exists, it's moved to the root of the tree allowing for constant time re-access.

##### Return value
Returns true if the item does exist, otherwise false.

##### Complexity
**Amortized O(log n)** time.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.contains(0)) //false, because 0 doesn't exist in the set
    System.print(set.insert(666)) //true
    System.print(set.contains(666)) //true, 666 does exist in the set

### **succ(item)**

Gets the successor of item if it exists.

##### Note
if item exists, it's moved to the root of the tree allowing for constant time re-access, but not the successor.
If item doesn't exist, the method returns `null`

##### Return value
Returns the value of the successor if it exists, otherwise `null`.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.insert(0)) //true
    System.print(set.insert(666)) //true
    System.print(set.succ(0)) //666
    System.print(set.succ(666)) //null, 666 has no successtor
    System.print(set.succ(6)) // null, 6 doesn't exist in the set

##### Complexity
**Amortized O(log n)** time.

### **pred(item)**

Gets the predecessor of item if it exists.

##### Note
if item exists, it's moved to the root of the tree allowing for constant time re-access, but not the predecessor.
If item doesn't exist, the method returns `null`

##### Return value
Returns the value of the predecessor if it exists, otherwise `null`.

##### Complexity
**Amortized O(log n)** time.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.insert(0)) //true
    System.print(set.insert(666)) //true
    System.print(set.pred(0)) //null, 0 has no predecessor
    System.print(set.pred(666)) //0
    System.print(set.pred(6)) // null, 6 doesn't exist in the set

### **clear()**

Clears the set

##### Return value
Returns `null`

##### Complexity
**O(1)** time.

### **splay(item)**

Moves item to the root of the tree, allowing for constant time re-access.

##### Note
Do not use this directly unless you have a good reason to.
If the item doesn't exist in the set, the tree structure is altered and the new item will not be added.

##### Return value
Returns true in all cases.

##### Complexity
**Amortized O(log n)** time.

## Getters

### **min**

Gets the minimal item in the set

##### Note

If the minimal item is found, it's moved to the root of the tree allowing for constant time re-access.

##### Return value
Returns `null` if the set is empty, otherwise the value of the minimal item.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.min) //null, set is empty
    System.print(set.insert(666)) //true
    System.print(set.insert(0)) // true
    System.print(set.min) // 0

##### Complexity
**Amortized O(log n)** time.

### **max**

Gets the maximal item in the set

##### Note

If the maximal item is found, it's moved to the root of the tree allowing for constant time re-access.

##### Return value
Returns `null` if the set is empty, otherwise the value of the maximal item.

##### Example
    :::wren
    var set = Set.new()
    System.print(set.max) //null, set is empty
    System.print(set.insert(666)) //true
    System.print(set.insert(0)) // true
    System.print(set.max) // 666

##### Complexity
**Amortized O(log n)** time.
