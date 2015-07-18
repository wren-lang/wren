^title IO Class
^category core

The IO class can be used to read and write to and from the console.

## Static Methods

### IO.**print**(objects...)

Prints a series of objects to the console followed by a newline. Each object is
converted to a string by calling `toString` on it. This is overloaded to
support up to 16 objects. To pass more, use `printAll()`.

  > IO.print("I like bananas")
  I like bananas
  > IO.print("Oranges", 10)
  Oranges10

### IO.**printAll**(sequence)

Iterates over [sequence] and prints each element, then prints a single newline
at the end. Each element is converted to a string by calling `toString` on it.

  > IO.printAll([1, [2, 3], 4])
  1[2, 3]4

### IO.**write**(object)

Prints a single value to the console, but does not print a newline character
afterwards. Converts the value to a string by calling `toString` on it.

  > IO.write(4 + 5)
  9>

In the above example, the result of `4 + 5` is printed, and then the prompt is
printed on the same line because no newline character was printed afterwards.

### IO.**read**()

Reads in a line of text from stdin. Note that the returned text includes the
trailing newline.

  > var name = IO.read()
  John
  > IO.print("Hello " + name + "!")
  Hello John
  !
  >

### IO.**read**(prompt)

Displays `prompt` then reads in a line of text from stdin. Note that the
returned text includes the trailing newline.

  > var name = IO.read("Enter your name: ")
  Enter your name: John
  > IO.print("Hello " + name + "!")
  Hello John
  !
  >
