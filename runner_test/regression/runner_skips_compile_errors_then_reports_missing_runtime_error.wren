// Regression test for the runner.
// After skipping >= 2 lines of compile errors, it would raise IndexError
// because there are no more error lines (now that the trailing last empty one
// is removed).
class
{
  foo
// expect runtime error: bar
