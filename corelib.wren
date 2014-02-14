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
    for (i in 0...this.count) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
    }
    result = result + "]"
    return result
  }

  + that {
    var newList = []
    if (this.count > 0) {
      for (element in this) {
        newList.add(element)
      }
    }
    if (that is Range || that.count > 0) {
      for (element in that) {
        newList.add(element)
      }
    }
    return newList
  }
}
