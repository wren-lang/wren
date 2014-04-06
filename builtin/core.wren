class Sequence {
  map(f) {
    var result = []
    for (element in this) {
      result.add(f.call(element))
    }
    return result
  }

  where(f) {
    var result = []
    for (element in this) {
      if (f.call(element)) result.add(element)
    }
    return result
  }
}

class List is Sequence {
  addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  toString {
    var result = "["
    for (i in 0...count) {
      if (i > 0) result = result + ", "
      result = result + this[i].toString
    }
    result = result + "]"
    return result
  }

  + that {
    var result = this[0..-1]
    for (element in that) {
      result.add(element)
    }
    return result
  }
}

class Range is Sequence {}
