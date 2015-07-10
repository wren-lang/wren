class Foo {
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

IO.print(Foo.new().thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
