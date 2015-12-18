class Returns {
  def foreign static implicitNull

  def foreign static returnInt
  def foreign static returnFloat

  def foreign static returnTrue
  def foreign static returnFalse

  def foreign static returnString
  def foreign static returnBytes
}

System.print(Returns.implicitNull == null) // expect: true

System.print(Returns.returnInt)  // expect: 123456
System.print(Returns.returnFloat) // expect: 123.456

System.print(Returns.returnTrue)  // expect: true
System.print(Returns.returnFalse) // expect: false

System.print(Returns.returnString) // expect: a string
System.print(Returns.returnBytes.bytes.toList) // expect: [97, 0, 98, 0, 99]
