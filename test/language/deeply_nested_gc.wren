var head

for (i in 1..400000) {
  head = { "next" : head }
}

System.print("done") // expect: done