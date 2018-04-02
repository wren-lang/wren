class Bool {}
class Fiber {}
class Fn {}
class Null {}
class Num {}

class Algorithm {
  // Non-modifying sequence operations.
  static all(seq, unary_predicate) {
    var result = true
    for (element in seq) {
      result = unary_predicate.call(element)
      if (!result) return result
    }
    return result
  }
  
  static any(seq, unary_predicate) {
    var result = false
    for (element in seq) {
      result = unary_predicate.call(element)
      if (result) return result
    }
    return result
  }
  
  static contains(seq, element) {
    for (item in seq) {
      if (element == item) return true
    }
    return false
  }
  
  static count(seq) {
    var result = 0
    for (element in seq) {
      result = result + 1
    }
    return result
  }

  static count(seq, unary_predicate) {
    var result = 0
    for (element in seq) {
      if (unary_predicate.call(element)) result = result + 1
    }
    return result
  }
  
  static each(seq, fn) {
    for (element in seq) {
      fn.call(element)
    }
  }
  
  static isEmpty(seq) { seq.iterate(null) ? false : true }
  
  // Modifying sequence operations.
  static copy(seq, res) {
    for (element in seq) {
      res.add(element)
    }
    return res
  }
  
  static copy(seq, res, unary_predicate) {
    for (element in seq) {
      if (unary_predicate.call(element)) res.add(element)
    }
    return res
  }
  
  // Views
  static map(seq, transformation) { MapSequence.new(seq, transformation) }
  
  static skip(seq, count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }
    return SkipSequence.new(seq, count)
  }

  static take(seq, count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }
    
    return TakeSequence.new(seq, count)
  }
  
  static where(seq, unary_predicate) { WhereSequence.new(seq, unary_predicate) }
  
  static reduce(seq, fn) {
    var iter = seq.iterate(null)
    if (!iter) Fiber.abort("Can't reduce an empty sequence.")
    
    // Seed with the first element.
    var result = seq.iteratorValue(iter)
    while (iter = seq.iterate(iter)) {
      result = fn.call(result, seq.iteratorValue(iter))
    }
    return result
  }
  
  static reduce(seq, acc, fn) {
    for (element in seq) {
      acc = fn.call(acc, element)
    }
    return acc
  }
  
  static join(seq) { join(seq, "") }
  
  static join(seq, sep) {
    var first = true
    var result = ""
    
    for (element in seq) {
      if (!first) result = result + sep
      first = false
      result = result + element.toString
    }
    return result
  }
  
  static toList(seq) { Algorithm.copy(seq, List.new()) }
}

class Sequence {
  all(unary_predicate)       { Algorithm.all(this, unary_predicate) }
  any(unary_predicate)       { Algorithm.any(this, unary_predicate) }
  contains(element)          { Algorithm.contains(this, element) }
  count                      { Algorithm.count(this) }
  count(unary_predicate)     { Algorithm.count(this, unary_predicate) }
  each(fn)                   { Algorithm.each(this, fn) }
  isEmpty                    { Algorithm.isEmpty(this) }

  map(transformation)        { Algorithm.map(this, transformation) }
  skip(count)                { Algorithm.skip(this, count) }
  take(count)                { Algorithm.take(this, count) }
  where(unary_predicate)     { Algorithm.where(this, unary_predicate) }
  reduce(fn)                 { Algorithm.reduce(this, fn) }
  reduce(acc, fn)            { Algorithm.reduce(this, acc, fn) }

  join()                     { Algorithm.join(this) }
  join(sep)                  { Algorithm.join(this, sep) }

  toList                     { Algorithm.toList(this) }
}

class MapSequence is Sequence {
  construct new(sequence, fn) {
    _sequence = sequence
    _fn = fn
  }

  iterate(iterator) { _sequence.iterate(iterator) }
  iteratorValue(iterator) { _fn.call(_sequence.iteratorValue(iterator)) }
}

class SkipSequence is Sequence {
  construct new(sequence, count) {
    _sequence = sequence
    _count = count
  }

  iterate(iterator) {
    if (iterator) {
      return _sequence.iterate(iterator)
    } else {
      iterator = _sequence.iterate(iterator)
      var count = _count
      while (count > 0 && iterator) {
        iterator = _sequence.iterate(iterator)
        count = count - 1
      }
      return iterator
    }
  }

  iteratorValue(iterator) { _sequence.iteratorValue(iterator) }
}

class TakeSequence is Sequence {
  construct new(sequence, count) {
    _sequence = sequence
    _count = count
  }

  iterate(iterator) {
    if (!iterator) _taken = 1 else _taken = _taken + 1
    return _taken > _count ? null : _sequence.iterate(iterator)
  }

  iteratorValue(iterator) { _sequence.iteratorValue(iterator) }
}

class WhereSequence is Sequence {
  construct new(sequence, fn) {
    _sequence = sequence
    _fn = fn
  }

  iterate(iterator) {
    while (iterator = _sequence.iterate(iterator)) {
      if (_fn.call(_sequence.iteratorValue(iterator))) break
    }
    return iterator
  }

  iteratorValue(iterator) { _sequence.iteratorValue(iterator) }
}

class String is Sequence {
  bytes { StringByteSequence.new(this) }
  codePoints { StringCodePointSequence.new(this) }

  split(delimiter) {
    if (!(delimiter is String) || delimiter.isEmpty) {
      Fiber.abort("Delimiter must be a non-empty string.")
    }

    var result = []

    var last = 0
    var index = 0

    var delimSize = delimiter.byteCount_
    var size = byteCount_

    while (last < size && (index = indexOf(delimiter, last)) != -1) {
      result.add(this[last...index])
      last = index + delimSize
    }

    if (last < size) {
      result.add(this[last..-1])
    } else {
      result.add("")
    }
    return result
  }

  replace(from, to) {
    if (!(from is String) || from.isEmpty) {
      Fiber.abort("From must be a non-empty string.")
    } else if (!(to is String)) {
      Fiber.abort("To must be a string.")
    }

    var result = ""

    var last = 0
    var index = 0

    var fromSize = from.byteCount_
    var size = byteCount_

    while (last < size && (index = indexOf(from, last)) != -1) {
      result = result + this[last...index] + to
      last = index + fromSize
    }

    if (last < size) result = result + this[last..-1]

    return result
  }

  *(count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }

    var result = ""
    for (i in 0...count) {
      result = result + this
    }
    return result
  }
}

class StringByteSequence is Sequence {
  construct new(string) {
    _string = string
  }

  [index] { _string.byteAt_(index) }
  iterate(iterator) { _string.iterateByte_(iterator) }
  iteratorValue(iterator) { _string.byteAt_(iterator) }

  count { _string.byteCount_ }
}

class StringCodePointSequence is Sequence {
  construct new(string) {
    _string = string
  }

  [index] { _string.codePointAt_(index) }
  iterate(iterator) { _string.iterate(iterator) }
  iteratorValue(iterator) { _string.codePointAt_(iterator) }

  count { _string.count }
}

class List is Sequence {
  addAll(other) {
    for (element in other) {
      add(element)
    }
    return other
  }

  toString { "[%(join(", "))]" }

  +(other) {
    var result = this[0..-1]
    for (element in other) {
      result.add(element)
    }
    return result
  }

  *(count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }

    var result = []
    for (i in 0...count) {
      result.addAll(this)
    }
    return result
  }
}

class Map is Sequence {
  keys { MapKeySequence.new(this) }
  values { MapValueSequence.new(this) }

  toString {
    var first = true
    var result = "{"

    for (key in keys) {
      if (!first) result = result + ", "
      first = false
      result = result + "%(key): %(this[key])"
    }

    return result + "}"
  }

  iteratorValue(iterator) {
    return MapEntry.new(
        keyIteratorValue_(iterator),
        valueIteratorValue_(iterator))
  }
}

class MapEntry {
  construct new(key, value) {
    _key = key
    _value = value
  }

  key { _key }
  value { _value }

  toString { "%(_key):%(_value)" }
}

class MapKeySequence is Sequence {
  construct new(map) {
    _map = map
  }

  iterate(n) { _map.iterate(n) }
  iteratorValue(iterator) { _map.keyIteratorValue_(iterator) }
}

class MapValueSequence is Sequence {
  construct new(map) {
    _map = map
  }

  iterate(n) { _map.iterate(n) }
  iteratorValue(iterator) { _map.valueIteratorValue_(iterator) }
}

class Range is Sequence {}

class System {
  static print() {
    writeString_("\n")
  }

  static print(obj) {
    writeObject_(obj)
    writeString_("\n")
    return obj
  }

  static printAll(sequence) {
    for (object in sequence) writeObject_(object)
    writeString_("\n")
  }

  static write(obj) {
    writeObject_(obj)
    return obj
  }

  static writeAll(sequence) {
    for (object in sequence) writeObject_(object)
  }

  static writeObject_(obj) {
    var string = obj.toString
    if (string is String) {
      writeString_(string)
    } else {
      writeString_("[invalid toString]")
    }
  }
}
