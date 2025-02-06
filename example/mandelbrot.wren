var yMin = -0.2
var yMax = 0.1
var xMin = -1.5
var xMax = -1.1

var COLS = 80
var ROWS = 25
var INSIDE = " " // or "\u2591" // LIGHT SHADE
var PALETTE = " .:;+=xX$&"

var ITERATIONS = 80

for (row in 0...ROWS) {
  var y = (row / (ROWS - 1)) * (yMax - yMin) + yMin

  for (col in 0...COLS) {
    var x = (col / (COLS - 1)) * (xMax - xMin) + xMin

    var char = INSIDE

    // Prepare to iterate.
    var xi = x
    var yi = y

    for (iter in 0...ITERATIONS) {
      // Square.
      var xs = (xi * xi) - (yi * yi)
      var ys = 2 * xi * yi

      // Add the seed.
      xi = xs + x
      yi = ys + y

      // Stop if the point escaped.
      var ds = (xi * xi) + (yi * yi)
      if (ds > 4) {
        char = PALETTE[(iter * PALETTE.count / ITERATIONS).floor]
        break
      }
    }

    System.write(char)
  }

  // The y axis grows downwards, but the set is symmetrical.
  System.print()
}
