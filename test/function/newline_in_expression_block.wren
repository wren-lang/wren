new Fn { IO.print("ok") // expect error
}.call // expect error

// The second error is cascaded here. If it starts failing, just remove that
// expectation.
