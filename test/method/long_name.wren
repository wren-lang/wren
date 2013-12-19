class Foo {
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

io.write((new Foo).thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
