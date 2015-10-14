var start = System.clock

var count = 0
for (i in 1..1000000) {
  if ("abc" == "abc") count = count + 1
  if ("a slightly longer string" ==
      "a slightly longer string") count = count + 1
  if ("a significantly longer string but still not overwhelmingly long string" ==
      "a significantly longer string but still not overwhelmingly long string") count = count + 1

  if ("" == "abc") count = count + 1
  if ("abc" == "abcd") count = count + 1
  if ("changed one character" == "changed !ne character") count = count + 1
  if ("123" == 123) count = count + 1
  if ("a slightly longer string" ==
      "a slightly longer string!") count = count + 1
  if ("a slightly longer string" ==
      "a slightly longer strinh") count = count + 1
  if ("a significantly longer string but still not overwhelmingly long string" ==
      "another") count = count + 1
}

System.print(count)
System.print("elapsed: %(System.clock - start)")
