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

class NthToggle is Toggle {
  this new(startState, maxCounter) super.new(startState) {
    _countMax = maxCounter
    _count = 0
  }

  activate {
    _count = _count + 1
    if (_count >= _countMax) {
      super.activate
      _count = 0
    }

    return this
  }
}

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
