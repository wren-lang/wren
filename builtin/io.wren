class IO {
  static print(obj) {
    IO.writeString_(obj.toString)
    IO.writeString_("\n")
    return obj
  }

  static write(obj) {
    IO.writeString_(obj.toString)
    return obj
  }
}
