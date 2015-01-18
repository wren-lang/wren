^title String Class
^category core

A string of Unicode code points stored in UTF-8.

### **contains**(other)

Checks if `other` is a substring of the string.

It is a runtime error if `other` is not a string.

### **count**

Returns the length of the string.

### **endsWith(suffix)**

Checks if the string ends with `suffix`.

It is a runtime error if `suffix` is not a string.

### **indexOf(search)**

Returns the index of `search` in the string or -1 if `search` is not a
substring of the string.

It is a runtime error if `search` is not a string.

### **startsWith(prefix)**

Checks if the string starts with `prefix`.

It is a runtime error if `prefix` is not a string.

### **+**(other) operator

Returns a new string that concatenates this string and `other`.

It is a runtime error if `other` is not a string.

### **==**(other) operator

Checks if the string is equal to `other`.

### **!=**(other) operator

Check if the string is not equal to `other`.

### **[**index**]** operator

Returns a one character string of the value at `index`.

It is a runtime error if `index` is greater than the length of the string.

*Note: This does not currently handle UTF-8 characters correctly.*
