// Note: This is converted to a C string literal and inserted into
// src/wren_core.c using make_corelib.
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

class List {
  toString {
    var result = "["
    var i = 0
    // TODO: Use for loop.
    while (i < this.count) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
      i = i + 1
    }
    result = result + "]"
    return result
  }
}
