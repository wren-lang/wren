class Foo {
  construct new() {}
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

System.print(Foo.new().thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
