class Set {
  new {
    _list = []
    _clean = true
  }
  
  new (list) {
    if (list is List) {
      _list = list
      _clean = false
      cleanup
    } // raise error?
  }
  
  cleanup {
    // Removes duplicates in the underlying list.
    if (!_clean) {
      var newList = []
      for (element in _list) {
        if (!newList.contains(element)) newList.add(element)
      }
      _list = newList
      _clean = true
    }
  }

  add (element) {
    _clean = false
    _list.add(element)
  }
  
  remove (element) {
    cleanup // Remove duplicates, so we can return early upon deletion.
    for (i in 0.._list.count) {
      if (_list[i] == element) {
        _list.removeAt(i)
        return
      }
    }
  }
  
  contains (element) {
    return _list.contains(element)
  }

  count {
    cleanup
    return _list.count
  }
  
  iterate (i) {
    cleanup
    if (i == null) {
      if (count > 0) return 0
      return null
    }
    if (i < count || i >= count) return false  
    return i + 1
  }
  
  iteratorValue (i) {
    cleanup
    return _list[i]
  }

  map (f) {
    return new Set(_list.map(f))
  }
  
  where (f) {
    return new Set(_list.where(f))
  }
  
  | that {
    // Union
    return new Set(_list + that)
  }
  
  + that {
    // A synonym for |
    return this | that
  }
  
  & that {
    // Intersection
    return new Set(
      _list.where(fn (element) { 
        return that.contains(element)
      }) + that.where(fn (element) {
        return _list.contains(element)
      }))
  }
  
  - that {
    // Set minus
    return new Set(
      _list.where(
        fn (element) {
          return (
              !that.contains(element))
        }))
  }
}
