class ForeignClass {
  construct new() {}
}

class Maps {
  foreign static newMap()
  foreign static insert()
  foreign static contains(map, key)
  foreign static contains()
  foreign static containsFalse()
  foreign static count()
  foreign static count(map)
  foreign static remove(map)
  foreign static invalidInsert(obj)
}

// map new + get/set API

var map = Maps.newMap()
System.print(map is Map) // expect: true
System.print(map.count) // expect: 0

var data = Maps.insert()
System.print(data["England"]) // expect: London
System.print(data["Empty"])   // expect: []
System.print(data[1.0])       // expect: 42
System.print(data[false])     // expect: true
System.print(data[null])      // expect: null

// remove API

var removed = Maps.remove({ "key":"value", "other":"data" })
System.print(removed) // expect: value

var removedNone = Maps.remove({})
System.print(removedNone) // expect: null

// count API

var countMap = { "key":"value", "other":"data", 4:"number key" }
System.print(Maps.count(countMap)) // expect: 3
Maps.remove(countMap) //remove using API
System.print(Maps.count(countMap)) // expect: 2
countMap.remove("other") //remove wren side
System.print(Maps.count(countMap)) // expect: 1

var countAPI = Maps.count()
System.print(countAPI) // expect: 5

//contains key API

var containsMap = { "key":"value", "other":"data", 4:"number key" }
System.print(Maps.contains(containsMap, "key")) // expect: true
System.print(Maps.contains(containsMap, "fake")) // expect: false
System.print(Maps.contains(containsMap, "other")) // expect: true

Maps.remove(containsMap) //remove using API
System.print(Maps.contains(containsMap, "key")) // expect: false

containsMap.remove("other") //remove wren side
System.print(Maps.contains(containsMap, "other")) // expect: false

System.print(Maps.contains()) // expect: true
System.print(Maps.containsFalse()) // expect: false
