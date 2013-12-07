class Foo {
  thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

io.write(Foo.new.thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
