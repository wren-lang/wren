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
  
  copy {
    var newList = []
    if (!isEmpty) {
      for (element in this) {
        newList.add(element)
      }
    }
    return newList
  }
  
  + that {
    var newList = this.copy
    if (!that.isEmpty) {
      for (element in that) {
        newList.add(element)
      }
    }
    return newList
  }
  
  isEmpty {
    return count == 0
  }
}

// class Range {
//   count {
//     return max - min
//   }
//  
//   isEmpty {
//     return count == 0
//   }
// }
