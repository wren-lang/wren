class Foo {
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

IO.write((new Foo).thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
