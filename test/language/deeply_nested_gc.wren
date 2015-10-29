var head

for (i in 1..400000) {
  head = { "next" : head }
}

System.gc()
System.print("done") // expect: done
