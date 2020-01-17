class Maps {
  foreign static newMap()
  foreign static insert()
}

var map = Maps.newMap()
System.print(map is Map) // expect: true
System.print(map.count) // expect: 0

var data = Maps.insert()
System.print(data["England"]) // expect: London
System.print(data["Empty"])   // expect: []
System.print(data[1.0])       // expect: 42
System.print(data[false]) // expect: true
System.print(data[null]) // expect: null
