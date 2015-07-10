var list = List.new()

IO.print(list.count) // expect: 0
IO.print(list) // expect: []
list.add(1)
IO.print(list) // expect: [1]
