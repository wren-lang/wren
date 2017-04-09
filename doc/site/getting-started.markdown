^title Getting Started

Getting Wren running on your machine is straightforward. Tiny C programs with
few dependencies are nice that way. "Wren" encompasses two separate artifacts:

 *  **The virtual machine.** This is the core chunk of C that executes Wren
    source code. It is just a library, not a standalone application. It's
    designed to be [embedded][] in a larger host application. It has no
    dependencies beyond the C standard library. You can use it as a static
    library, shared library, or simply compile the source into your app.

 *  **The command line executable.** Wren also ships with a CLI wrapper around
    the VM. This gives you a way to run Wren code from the command-line, and
    also includes modules for talking to the operating system&mdash;file IO,
    networking, stuff like that. It depends on [libuv][] for that
    functionality.

[embedded]: embedding
[libuv]: http://libuv.org/

If you're on a Unix or Mac and you can rock a command line, it's just:

    :::sh
    $ git clone https://github.com/munificent/wren.git
    $ cd wren
    $ make
    $ ./wren

This builds both the VM and the CLI. The release build of the CLI goes right
into the repo's top level directory. Binaries for other configurations are built
to `bin/`. Static and shared libraries for embedding Wren get built in `lib/`.

For Mac users, there is also an XCode project under `util/xcode`. For
Windows brethren, `util/msvc2013` contains a Visual Studio solution. Note
that these may not have the exact same build settings as the makefile. The
makefile is the "official" way to compile Wren.

If you only want to build the VM, you can do:

    :::sh
    $ make vm

This compiles the VM to static and shared libraries.

## Interactive mode

If you just run `wren` without any arguments, it starts the interpreter in
interactive mode. You can type in a line of code, and it immediately executes
it. Here's something to try:

    :::wren
    System.print("Hello, world!")

Or a little more exciting:

    :::wren
    for (i in 1..10) System.print("Counting up %(i)")

You can exit the interpreter using good old Ctrl-C or Ctrl-D, or just throw
your computer to the ground and storm off.

## Running scripts

The standalone interpreter can also load scripts from files and run them. Just
pass the name of the script to `wren`. Create a file named "my_script.wren" in
your favorite text editor and paste this into it:

    :::wren
    for (yPixel in 0...24) {
      var y = yPixel / 12 - 1
      for (xPixel in 0...80) {
        var x = xPixel / 30 - 2
        var x0 = x
        var y0 = y
        var iter = 0
        while (iter < 11 && x0 * x0 + y0 * y0 <= 4) {
          var x1 = (x0 * x0) - (y0 * y0) + x
          var y1 = 2 * x0 * y0 + y
          x0 = x1
          y0 = y1
          iter = iter + 1
        }
        System.write(" .-:;+=xX$& "[iter])
      }

      System.print("")
    }

Now run:

    :::sh
    $ ./wren my_script.wren

Neat, right? You're a Wren programmer now! The next step is to [learn the
language](syntax.html). If you run into bugs, or have ideas or questions, any of
the following work:

 *  **Ask on the [Wren mailing list][list].**
 *  Tell me on twitter at [@munificentbob][twitter].
 *  [File a ticket][issue] at [the GitHub repo][repo].
 *  Send a pull request.
 *  Email me at [`robert@stuffwithstuff.com`](mailto:robert@stuffwithstuff.com).

[list]: https://groups.google.com/forum/#!forum/wren-lang
[twitter]: https://twitter.com/intent/user?screen_name=munificentbob
[issue]: https://github.com/munificent/wren/issues
[repo]: https://github.com/munificent/wren
