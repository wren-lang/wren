class List {
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
