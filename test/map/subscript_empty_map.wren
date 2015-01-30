// This is a regression test to ensure map handles a null entry array.

var map = {}
IO.print(map["key"]) // expect: null
