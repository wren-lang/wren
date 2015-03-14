for (i in 0..10) {
  IO.print(i)

  {
    var a = "a"
    {
      var b = "b"
      {
        var c = "c"
        if (i > 1) break
      }
    }
  }
}

// expect: 0
// expect: 1
// expect: 2
