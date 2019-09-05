^title Map Class

An associative collection that maps keys to values. More details [here](../../maps.html).

## Methods

### **clear**()

Removes all entries from the map.

### **containsKey**(key)

Returns `true` if the map contains `key` or `false` otherwise.

### **count**

The number of entries in the map.

### **keys**

A [Sequence](sequence.html) that can be used to iterate over the keys in the
map. Note that iteration order is undefined. All keys will be iterated over,
but may be in any order, and may even change between invocations of Wren.

### **remove**(key)

Removes `key` and the value associated with it from the map. Returns the value.

If the key was not present, returns `null`.

### **values**

A [Sequence](sequence.html) that can be used to iterate over the values in the
map. Note that iteration order is undefined. All values will be iterated over,
but may be in any order, and may even change between invocations of Wren.

If multiple keys are associated with the same value, the value will appear
multiple times in the sequence.

### **[**key**]** operator

Gets the value associated with `key` in the map. If `key` is not present in the
map, returns `null`.

    :::wren
    var map = {"george": "harrison", "ringo": "starr"}
    System.print(map["ringo"]) //> starr
    System.print(map["pete"])  //> null

### **[**key**]=**(value) operator

Associates `value` with `key` in the map. If `key` was already in the map, this
replaces the previous association.

It is a runtime error if the key is not a [Bool](bool.html),
[Class](class.html), [Null](null.html), [Num](num.html), [Range](range.html),
or [String](string.html).
