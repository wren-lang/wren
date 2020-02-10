class VM {
  foreign static nullConfig()
  foreign static multipleInterpretCalls()
}
// TODO: Other configuration settings.

System.print(VM.nullConfig()) // expect: true
System.print(VM.multipleInterpretCalls()) // expect: true
