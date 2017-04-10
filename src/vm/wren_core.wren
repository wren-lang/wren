class Bool {}
class Fiber {}
class Fn {}
class Null {}
class Num {}

class Sequence {
  all(f) {
    var result = true
    for (element in this) {
      result = f.call(element)
      if (!result) return result
    }
    return result
  }

  any(f) {
    var result = false
    for (element in this) {
      result = f.call(element)
      if (result) return result
    }
    return result
  }

  contains(element) {
    for (item in this) {
      if (element == item) return true
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
      if (f.call(element)) result = result + 1
    }
    return result
  }

  each(f) {
    for (element in this) {
      f.call(element)
    }
  }

  isEmpty { iterate(null) ? false : true }

  map(transformation) { MapSequence.new(this, transformation) }

  skip(count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }

    return SkipSequence.new(this, count)
  }

  take(count) {
    if (!(count is Num) || !count.isInteger || count < 0) {
      Fiber.abort("Count must be a non-negative integer.")
    }

    return TakeSequence.new(this, count)
  }

  where(predicate) { WhereSequence.new(this, predicate) }

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

  join() { join("") }

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

  toList {
    var result = List.new()
    for (element in this) {
      result.add(element)
    }
    return result
  }
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

class Set {
  construct new() {
      _root = null
      _header = __SetNode.new(null) //avoid recreation
  }

  insert(value) {
      if ( !_root ) {
          _root = __SetNode.new(value)
          return true
      }

      splay(value)
      if ( _root.data == value ) return false //already there

      var node = __SetNode.new(value)
      if ( value < _root.data ) {
          node.left = _root.left
          node.right = _root
          _root.left = null
      } else {
          node.right = _root.right
          node.left = _root
          _root.right = null
      }

      _root = node
      return true
  }

  remove(value) {
      if ( !_root ) return false
      
      splay(value)
      if ( value != _root.data ) return false //not there

      //delete the root
      if ( !_root.left ) {
          _root = _root.right
      } else {
          var node = _root.right
          _root = _root.left
          splay(value)
          _root.right = node
      }

      return true
  }

  splay(value) {
      var left = _header
      var right = _header
      var node = _root

      _header.left = _header.right = null
      
      while ( true ) {
          if ( value < node.data ) {
              if ( !node.left ) break

              if ( value < node.left.data ) {
                  var y = node.left
                  node.left = y.right
                  y.right = node
                  node = y

                  if ( !node.left ) break
              }

              right.left = node
              right = node
              node = node.left
          } else if ( value == node.data ) {
              break
          } else {
              if ( !node.right ) break

              if ( value > node.right.data ) {
                  var y = node.right
                  node.right = y.left
                  y.left = node
                  node = y

                  if ( !node.right ) break
              }

              left.right = node
              left = node
              node = node.right
          }
      }

      left.right = node.left
      right.left = node.right
      node.left = _header.right
      node.right = _header.left
      _root = node
      return true
  }

  contains(value) {
      if ( !_root ) return false
      
      splay(value)
      if ( _root.data != value ) {
        return false
      } else {
        return true
      }
  }

  max {
      if ( !_root ) return null

      var node = _root
      while ( node.right ) node = node.right
      splay(node.data)
      return node.data
  }

  min {
      if ( !_root ) return null

      var node = _root
      while ( node.left ) node = node.left
      splay(node.data)
      return node.data
  }


  empty {
      if ( _root ) {
          return false
      } else {
          return true
      }
  }

  //I think calling like a function is better
  clear() {
      _root = null
  }

  succ(value) {
      if ( !_root ) return null
      
      splay(value)
      if ( _root.data != value || !_root.right ) {
          return null
      } else {
          var node = _root.right
          while ( node.left ) node = node.left

          return node.data
      }
  }

  pred(value) {
      if ( !_root ) return null
      
      splay(value)
      if ( _root.data != value || !_root.left ) {
          return null
      } else {
          var node = _root.left
          while ( node.right ) node = node.right

          return node.data
      }
  }
  
  static init() {
      class SetNode {
          left { _left }
          right { _right }
          data { _data }

          left=(left) { _left = left }
          right=(right) { _right = right }
          data=(data) { _data }

          construct new(data) {
              _data = data
          }
      }

      __SetNode = SetNode
  }
}
Set.init()
