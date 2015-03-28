class Sequence {
  contains(element) {
    for (item in this) {
      if (element == item) {
        return true
      }
    }
    return false
  }

  count {
    var result = 0
    for (element in this) {
      result = result + 1
    }
    return result
  }

  count(f) {
    var result = 0
    for (element in this) {
      if (f.call(element)) {
        result = result + 1
      }
    }
    return result
  }

  map(transformation) { new MapSequence(this, transformation) }

  where(predicate) { new WhereSequence(this, predicate) }

  all(f) {
    for (element in this) {
      if (!f.call(element)) return false
    }
    return true
  }

  any(f) {
    for (element in this) {
      if (f.call(element)) return true
    }
    return false
  }

  reduce(acc, f) {
    for (element in this) {
      acc = f.call(acc, element)
    }
    return acc
  }

  reduce(f) {
    var iter = iterate(null)
    if (!iter) Fiber.abort("Can't reduce an empty sequence.")

    // Seed with the first element.
    var result = iteratorValue(iter)
    while (iter = iterate(iter)) {
      result = f.call(result, iteratorValue(iter))
    }

    return result
  }

  join { join("") }

  join(sep) {
    var first = true
    var result = ""

    for (element in this) {
      if (!first) result = result + sep
      first = false
      result = result + element.toString
    }

    return result
  }

  list {
    var result = new List
    for (element in this) {
      result.add(element)
    }
    return result
  }
}

class MapSequence is Sequence {
  new(seq, f) {
    _seq = seq
    _f = f
  }

  iterate(n) { _seq.iterate(n) }
  iteratorValue(iterator) { _f.call(_seq.iteratorValue(iterator)) }
}

class WhereSequence is Sequence {
  new(seq, f) {
    _seq = seq
    _f = f
  }

  iterate(n) {
    while (n = _seq.iterate(n)) {
      if (_f.call(_seq.iteratorValue(n))) break
    }
    return n
  }

  iteratorValue(iterator) { _seq.iteratorValue(iterator) }
}

class String is Sequence {
  bytes { new StringByteSequence(this) }
}

class StringByteSequence is Sequence {
  new(string) {
    _string = string
  }

  [index] { _string.byteAt(index) }
  iterate(iterator) { _string.iterateByte_(iterator) }
  iteratorValue(iterator) { _string.byteAt(iterator) }
}

class List is Sequence {
  addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  toString { "[" + join(", ") + "]" }

  +(other) {
    var result = this[0..-1]
    for (element in other) {
      result.add(element)
    }
    return result
  }
}

class Map {
  keys { new MapKeySequence(this) }
  values { new MapValueSequence(this) }

  toString {
    var first = true
    var result = "{"

    for (key in keys) {
      if (!first) result = result + ", "
      first = false
      result = result + key.toString + ": " + this[key].toString
    }

    return result + "}"
  }
}

class MapKeySequence is Sequence {
  new(map) {
    _map = map
  }

  iterate(n) { _map.iterate_(n) }
  iteratorValue(iterator) { _map.keyIteratorValue_(iterator) }
}

class MapValueSequence is Sequence {
  new(map) {
    _map = map
  }

  iterate(n) { _map.iterate_(n) }
  iteratorValue(iterator) { _map.valueIteratorValue_(iterator) }
}

class Range is Sequence {}
