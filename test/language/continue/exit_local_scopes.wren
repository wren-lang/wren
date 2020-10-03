for (i in 0..5) {
  {
    var a = "a"
    {
      var b = "b"
      {
        var c = "c"
        if (i == 1) continue
      }
    }
  }
  
  System.print(i)
}

// expect: 0
// expect: 2
// expect: 3
// expect: 4
// expect: 5
