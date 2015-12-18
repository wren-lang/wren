class Bool {}
class Fiber {}
class Fn {}
class Null {}
class Num {}

class Sequence {
  def all(f) {
    var result = true
    for (element in this) {
      result = f.call(element)
      if (!result) return result
    }
    return result
  }

  def any(f) {
    var result = false
    for (element in this) {
      result = f.call(element)
      if (result) return result
    }
    return result
  }

  def contains(element) {
    for (item in this) {
      if (element == item) return true
    }
    return false
  }

  def count {
    var result = 0
    for (element in this) {
      result = result + 1
    }
    return result
  }

  def count(f) {
    var result = 0
    for (element in this) {
      if (f.call(element)) result = result + 1
    }
    return result
  }

  def each(f) {
    for (element in this) {
      f.call(element)
    }
  }

  def isEmpty { iterate(null) ? false : true }

  def map(transformation) { MapSequence.new(this, transformation) }

  def where(predicate) { WhereSequence.new(this, predicate) }

  def reduce(acc, f) {
    for (element in this) {
      acc = f.call(acc, element)
    }
    return acc
  }

  def reduce(f) {
    var iter = iterate(null)
    if (!iter) Fiber.abort("Can't reduce an empty sequence.")

    // Seed with the first element.
    var result = iteratorValue(iter)
    while (iter = iterate(iter)) {
      result = f.call(result, iteratorValue(iter))
    }

    return result
  }

  def join() { join("") }

  def join(sep) {
    var first = true
    var result = ""

    for (element in this) {
      if (!first) result = result + sep
      first = false
      result = result + element.toString
    }

    return result
  }

  def toList {
    var result = List.new()
    for (element in this) {
      result.add(element)
    }
    return result
  }
}

class MapSequence is Sequence {
  def construct new(sequence, fn) {
    _sequence = sequence
    _fn = fn
  }

  def iterate(iterator) { _sequence.iterate(iterator) }
  def iteratorValue(iterator) { _fn.call(_sequence.iteratorValue(iterator)) }
}

class WhereSequence is Sequence {
  def construct new(sequence, fn) {
    _sequence = sequence
    _fn = fn
  }

  def iterate(iterator) {
    while (iterator = _sequence.iterate(iterator)) {
      if (_fn.call(_sequence.iteratorValue(iterator))) break
    }
    return iterator
  }

  def iteratorValue(iterator) { _sequence.iteratorValue(iterator) }
}

class String is Sequence {
  def bytes { StringByteSequence.new(this) }
  def codePoints { StringCodePointSequence.new(this) }
}

class StringByteSequence is Sequence {
  def construct new(string) {
    _string = string
  }

  def [index] { _string.byteAt_(index) }
  def iterate(iterator) { _string.iterateByte_(iterator) }
  def iteratorValue(iterator) { _string.byteAt_(iterator) }

  def count { _string.byteCount_ }
}

class StringCodePointSequence is Sequence {
  def construct new(string) {
    _string = string
  }

  def [index] { _string.codePointAt_(index) }
  def iterate(iterator) { _string.iterate(iterator) }
  def iteratorValue(iterator) { _string.codePointAt_(iterator) }

  def count { _string.count }
}

class List is Sequence {
  def addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  def toString { "[%(join(", "))]" }

  def +(other) {
    var result = this[0..-1]
    for (element in other) {
      result.add(element)
    }
    return result
  }
}

class Map {
  def keys { MapKeySequence.new(this) }
  def values { MapValueSequence.new(this) }

  def toString {
    var first = true
    var result = "{"

    for (key in keys) {
      if (!first) result = result + ", "
      first = false
      result = result + "%(key): %(this[key])"
    }

    return result + "}"
  }
}

class MapKeySequence is Sequence {
  def construct new(map) {
    _map = map
  }

  def iterate(n) { _map.iterate_(n) }
  def iteratorValue(iterator) { _map.keyIteratorValue_(iterator) }
}

class MapValueSequence is Sequence {
  def construct new(map) {
    _map = map
  }

  def iterate(n) { _map.iterate_(n) }
  def iteratorValue(iterator) { _map.valueIteratorValue_(iterator) }
}

class Range is Sequence {}

class System {
  def static print() {
    writeString_("\n")
  }

  def static print(obj) {
    writeObject_(obj)
    writeString_("\n")
    return obj
  }

  def static printAll(sequence) {
    for (object in sequence) writeObject_(object)
    writeString_("\n")
  }

  def static write(obj) {
    writeObject_(obj)
    return obj
  }

  def static writeAll(sequence) {
    for (object in sequence) writeObject_(object)
  }

  def static writeObject_(obj) {
    var string = obj.toString
    if (string is String) {
      writeString_(string)
    } else {
      writeString_("[invalid toString]")
    }
  }
}
