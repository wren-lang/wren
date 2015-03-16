class Sequence {
  count {
    var result = 0
    for (element in this) {
      result = result + 1
    }
    return result
  }

  map(f) { new MapSequence(this, f) }
  where(predicate) { new WhereSequence(this, predicate) }

  skip(count) { new SkipSequence(this, count) }
  take(count) { new TakeSequence(this, count) }

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

  toList() {
    var result = new List
    for (element in this) {
      result.add(element)
    }
    return result
  }
}

class String is Sequence {}

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

  contains(element) {
    for (item in this) {
      if (element == item) {
        return true
      }
    }
    return false
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
    n = _seq.iterate(n)
    while (n && !_f.call(_seq.iteratorValue(n))) {
      n = _seq.iterate(n)
    }
    return n
  }
  iteratorValue(iterator) { _seq.iteratorValue(iterator) }
}

class SkipSequence is Sequence {
  new(seq, count) {
    _seq = seq
    _count = count
  }

  iterate(n) {
    if (n) {
      return _seq.iterate(n)
    } else {
      n = _seq.iterate(n)
      var count = _count
      while (count > 0 && n) {
        n = _seq.iterate(n)
        count = count - 1
      }
      return n
    }
  }
  iteratorValue(iterator) { _seq.iteratorValue(iterator) }
}

class TakeSequence is Sequence {
  new(seq, count) {
    _seq = seq
    _count = count
  }

  iterate(n) {
    if (!n) _taken = 1 else _taken = _taken + 1
    return _taken > _count ? null : _seq.iterate(n)
  }
  iteratorValue(iterator) { _seq.iteratorValue(iterator) }
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
