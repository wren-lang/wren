import "get_variable_module"

class GetVariable {
  foreign static def beforeDefined()
  foreign static def afterDefined()
  foreign static def afterAssigned()
  foreign static def otherSlot()
  foreign static def otherModule()
}

System.print(GetVariable.beforeDefined()) // expect: null

var A = "a"

System.print(GetVariable.afterDefined()) // expect: a

A = "changed"

System.print(GetVariable.afterAssigned()) // expect: changed

var B = "b"
System.print(GetVariable.otherSlot()) // expect: b

System.print(GetVariable.otherModule()) // expect: value
