class Returns {
  foreign static def implicitNull

  foreign static def returnInt
  foreign static def returnFloat

  foreign static def returnTrue
  foreign static def returnFalse

  foreign static def returnString
  foreign static def returnBytes
}

System.print(Returns.implicitNull == null) // expect: true

System.print(Returns.returnInt)  // expect: 123456
System.print(Returns.returnFloat) // expect: 123.456

System.print(Returns.returnTrue)  // expect: true
System.print(Returns.returnFalse) // expect: false

System.print(Returns.returnString) // expect: a string
System.print(Returns.returnBytes.bytes.toList) // expect: [97, 0, 98, 0, 99]
