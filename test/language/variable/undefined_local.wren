// Error is on last line because this assumes "notDefined" is an implicitly
// defined global until it reaches the end of the file.
fn () {
  System.print(notDefined)  // expect error line 6
}()
