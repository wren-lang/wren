// Stops as soon as it finds a wren_modules directory, regardless of whether or
// not it contains the desired module.
import "foo" // expect runtime error: Could not load module 'foo'.
