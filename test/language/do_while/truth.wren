// False and null are false.
var i = 0
do {
  if(i != 0) {
	System.print("bad")
	break
  }
  System.print("false") // expect: false
  i = i + 1
} while (false)

i = 0
do {
  if(i != 0) {
	System.print("bad")
	break
  }
  System.print("null") // expect: null
  i = i + 1
} while (null)

// Everything else is true.
do {
  System.print("true") // expect: true
  break
} while (true)

do {
  System.print(0) // expect: 0
  break
} while (0)

do {
  System.print("string") // expect: string
  break
} while ("")
