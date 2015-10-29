var found = []
for (i in 1..1000) {
  var foo = 1337
  for (i in 1..1000) {
    foo = { "a" : foo, "b": foo }
  }
  var bar = foo
  for (i in 1..1000) {
    bar = bar["a"]
  }
  found.add(bar)
}

System.gc()
System.print(found.all {|i| i == 1337}) // expect: true
System.print("done") // expect: done
