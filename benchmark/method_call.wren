class Toggle {
  this new(startState) {
    _state = startState
  }

  value { return _state }
  activate {
    _state = !_state
    return this
  }
}

class NthToggle {
  this new(startState, maxCounter) {
    _state = startState
    _countMax = maxCounter
    _count = 0
  }

  value { return _state }

  activate {
    _count = _count + 1
    if (_count >= _countMax) {
      _state = !_state
      _count = 0
    }

    return this
  }
}

// TODO: The follow the other examples, we should be using inheritance here.
// Since Wren doesn't currently support inherited fields or calling superclass
// constructors, it doesn't. It probably won't make a huge perf difference,
// but it should be fixed when possible to be:
/*
class NthToggle is Toggle {
  this new(startState, maxCounter) {
    // TODO: Need to distinguish superclass method calls from superclass
    // constructor calls.
    super.new(startState)
    _countMax = maxCounter
    _count = 0
  }

  activate {
    _count = _count + 1
    if (_count >= _countMax) {
      _state = !_state
      _count = 0
    }

    return this
  }
}
*/

var start = OS.clock
var n = 1000000
var i = 0
var val = true
var toggle = Toggle.new(val)

while (i < n) {
  val = toggle.activate.value
  i = i + 1
}

io.write(toggle.value)

val = true
var ntoggle = NthToggle.new(val, 3)

i = 0
while (i < n) {
  val = ntoggle.activate.value
  i = i + 1
}

io.write(ntoggle.value)

io.write("elapsed: " + (OS.clock - start).toString)
