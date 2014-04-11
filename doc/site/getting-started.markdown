^title Getting Started

Getting Wren up and running on your machine should be pretty straightforward. Tiny C programs with no dependencies are nice that way. If you're on a Unix or Mac and you can rock a command line, it's just:

    :::bash
    $ git clone https://github.com/munificent/wren.git
    $ cd wren
    $ make
    $ ./wren

For Mac users, there is also an XCode project in the repo that you can use to hack on Wren. That's what I develop in. It builds fine from there but *may* not have the exact same build settings. The Makefile is the canonical way to compile it.

For our Windows bretheren, there's still a little work to be done. Ideally, the repo would include a Visual Studio solution for building Wren. I don't have a Windows machine, but if you do, I would be delighted to take a patch for this.

## Interactive mode

The above instructions will drop you into Wren's standalone interpreter in interactive mode. You can type a line of code in, and it will immediately execute it. Here's something to try:

    :::dart
    IO.print("Hello, world!")

Or a little more exciting:

    for (i in 1..10) IO.print("Counting up ", i)

You can exit the interpreter using good old Ctrl-C or Ctrl-D, or just throw your computer to the ground and storm off.

## Running scripts

The standalone interpreter can also load scripts from files and run them. Just pass the name of the script to wren. Create a file named `my_first_script.wren` in your favorite text editor and paste this into it:

    :::dart
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
        IO.write(" .-:;+=xX$& "[iter])
      }

      IO.print("")
    }

Now run:

    :::bash
    $ ./wren my_first_script.wren

Neat, right? You're a Wren programmer now! The next step is to [read more docs](syntax.html) and learn your way around the language. If you run into bugs, or have ideas or questions, any and all of the following work:

 *  Tell me on twitter at [`@munificentbob`](https://twitter.com/intent/user?screen_name=munificentbob).
 *  [File a ticket](https://github.com/munificent/wren/issues) at [the GitHub repo](https://github.com/munificent/wren).
 *  Send a pull request.
 *  Email me at [`bob@stuffwithstuff.com`](mailto:bob@stuffwithstuff.com).
