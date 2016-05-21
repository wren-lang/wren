^title Process Class

The Process class lets you work with operating system processes, including the
currently running one.

## Static Methods

### **allArguments**

The list of command-line arguments that were passed when the Wren process was
spawned. This includes the Wren executable itself, the path to the file being
run (if any), and any other options passed to Wren itself.

If you run:

    :::bash
    $ wren file.wren arg

This returns:

    :::wren
    System.print(Process.allArguments) //> ["wren", "file.wren", "arg"]

### **arguments**

The list of command-line arguments that were passed to your program when the
Wren process was spawned. This does not include arguments handled by Wren
itself.

If you run:

    :::bash
    $ wren file.wren arg

This returns:

    :::wren
    System.print(Process.arguments) //> ["arg"]
