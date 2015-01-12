var a = IO.read("a:") // stdin: first
var b = IO.read("b:") // stdin: second
IO.print
// Using write() here because the read lines include the trailing newline.
IO.write(a)
IO.write(b)

// Since stdin isn't echoed back to stdout, we don't see the input lines here,
// and there is no newline between the two prompts since that normally comes
// from the input itself.
// expect: a:b:
// expect: first
// expect: second