^title Wren CLI Usage

---


You can [download a build for your OS from the releases page](https://github.com/wren-lang/wren-cli/releases).

### Interactive mode

If you just run `wren_cli` without any arguments, it starts the interpreter in
interactive mode, where you can type in a line of code, and it immediately executes
it. You can exit the interpreter using good old Ctrl-C or Ctrl-D.

Here's something to try:

<pre class="snippet">
System.print("Hello, world!")
</pre>

Or a little more exciting:

<pre class="snippet">
for (i in 1..10) System.print("Counting up %(i)")
</pre>

### Running scripts

The standalone interpreter can also load scripts from files and run them. Just
pass the name of the script to `wren_cli`. Create a file named "my_script.wren" in
your favorite text editor and paste this into it:

<pre class="snippet">
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
</pre>

Now run:

    $ ./wren_cli my_script.wren
