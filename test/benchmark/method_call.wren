class Toggle {
  this new(startState) {
    _state = startState
  }

  value { _state }
  activate {
    _state = !_state
    return this
  }
}

class NthToggle is Toggle {
  this new(startState, maxCounter) {
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

var start = IO.clock
var n = 100000
var val = true
var toggle = Toggle.new(val)

for (i in 0...n) {
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
  val = toggle.activate.value
}

IO.print(toggle.value)

val = true
var ntoggle = NthToggle.new(val, 3)

for (i in 0...n) {
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
  val = ntoggle.activate.value
}

IO.print(ntoggle.value)
IO.print("elapsed: ", IO.clock - start)
