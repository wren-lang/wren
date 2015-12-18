class Foo {
  def thisIsAMethodNameThatExceedsTheMaximumNameLengthOf64CharactersBy1 { // expect error
    "body"
  }
}
