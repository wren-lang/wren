class Foo {
  construct new() {}
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

IO.print(Foo.new().thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
