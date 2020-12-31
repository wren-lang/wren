^title Timer Class

## Static Method

### Timer.**sleep**(milliseconds)

Suspends the current fiber for the given number of milliseconds. It is a runtime error if this is not a non-negative number.

This method is often used in conjunction with the Scheduler class which runs any scheduled tasks in separate fibers whilst the current fiber is sleeping.

Note that this method also suspends the System.clock method which will not give the correct running time for a program as a result.

