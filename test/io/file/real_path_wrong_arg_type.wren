import "io" for File

File.realPath(123) // expect runtime error: Path must be a string.

// TODO: Write success case tests too when we have an API to create symlinks
// from Wren.
