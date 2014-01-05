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

class Range {
  new(min, max) {
    _min = min
    _max = max
  }

  min { return _min }
  max { return _max }

  iterate(previous) {
    if (previous == null) return _min
    if (previous == _max) return false
    return previous + 1
  }

  iteratorValue(iterator) {
    return iterator
  }
}

class Num {
  .. other { return new Range(this, other) }
  ... other { return new Range(this, other - 1) }
}
