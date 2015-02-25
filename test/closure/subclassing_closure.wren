var f = null

{
  var a = "a"
  f = new Fn {
    IO.print(a)
  }
}
//@FIXME: repair the segfault create a propper test for the subclassing

//var closureSegfault = f
//class SubClosureSegfault is closureSegfault {} 

//var closureOk = f.call
//class SubClosureOk is closureOk {} 

