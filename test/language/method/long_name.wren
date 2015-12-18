class Foo {
  def construct new() {}
  def thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum {
    return "result"
  }
}

System.print(Foo.new().thisHasAMethodNameThatIsExactly64CharactersLongWhichIsTheMaximum) // expect: result
