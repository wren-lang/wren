^title IO Class
^category core

The IO class can be used to read and write to and from the console.

## Static Methods

### IO.**print**(objects...)

Prints any number of things to the console and then prints a newline
character. If you don't pass it a string, it will be converted to a string for
you. When passed multiple things, Wren outputs one after another.


	> IO.print("I like bananas")
	I like bananas
	> IO.print("Oranges", 10)
	Oranges10
	>

### IO.**write**(object)

Prints a single thing to the console, but does not print a newline character
afterwards. If you pass it something that isn't a string, it will convert it to
a string.

	> IO.write(4 + 5)
	9>

In the above example, the result of `4 + 5` is printed, and then the prompt is
printed on the same line because no newline character was printed afterwards.

### IO.**read**(prompt)

Reads in and returns a line of text from the console. Takes a single string to
be used as a prompt. Pass an empty string for no prompt. Note that the returned
line of text includes the newline character at the end.

	> var name = IO.read("Enter your name: ")
	Enter your name: John
	> IO.print("Hello " + name + "!")
	Hello John
	!
	>
