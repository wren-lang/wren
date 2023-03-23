var start = System.clock

var string1 = "some random string value"
var string2 = "some random string value"
for (i in 1..100000) {
  "%(string1)"
  "%(string1) "
  " %(string1)"
  " %(string1) "
  "%(string2)"
  "%(string2) "
  " %(string2)"
  " %(string2) "

  "%(string1)%(string2)"
  "%(string1)%(string2) "
  "%(string1) %(string2)"
  "%(string1) %(string2) "
  " %(string1)%(string2)"
  " %(string1)%(string2) "
  " %(string1) %(string2)"
  " %(string1) %(string2) "
}

System.print() // Make benchmark.py happy
System.print("elapsed: %(System.clock - start)")
