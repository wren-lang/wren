var i = 0
while (i <= 2) {
  i = i + 1

  System.print("outer %(i)")
  if (i == 2) continue

  var j = 0
  while (j <= 2) {
    j = j + 1
  
	if(j == 2) continue
    System.print("inner %(j)")
  }
}

// expect: outer 1
// expect: inner 1
// expect: inner 3
// expect: outer 2
// expect: outer 3
// expect: inner 1
// expect: inner 3