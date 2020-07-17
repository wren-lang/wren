This contains the Wren source code. It is organized like so:

*   `include`: the public header directory for the VM. If you are embedding the
    VM in your own application, you will add this to your include path.

*   `vm`: the source code for the Wren VM itself. If you are embedding the VM in
    your own application from source, you will compile the files here into your
    app.

*   `optional`: the Wren and C source code for the optional modules. These are
    built in to the VM and can be used even when you embed the VM in your own
    application. But they are also optional and can be compiled out by setting
    defines.
