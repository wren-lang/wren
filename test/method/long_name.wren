class Foo {
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

IO.print((new Foo).thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
