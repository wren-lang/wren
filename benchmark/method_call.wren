class Toggle {
  new(startState) {
    _state = startState
  }

  value { return _state }
  activate {
    _state = !_state
    return this
  }
}

class NthToggle is Toggle {
  new(startState, maxCounter) {
    super(startState)
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
var toggle = new Toggle(val)

while (i < n) {
  val = toggle.activate.value
  i = i + 1
}

IO.write(toggle.value)

val = true
var ntoggle = new NthToggle(val, 3)

i = 0
while (i < n) {
  val = ntoggle.activate.value
  i = i + 1
}

IO.write(ntoggle.value)

IO.write("elapsed: " + (OS.clock - start).toString)
