class Toggle {
  var _state;

  Toggle(startState) {
    _state = startState;
  }

  get value => _state;

  activate() {
    _state = !_state;
    return this;
  }
}

class NthToggle extends Toggle {
  var _count;
  var _countMax;

  NthToggle(startState, maxCounter)
      : super(startState) {
    _countMax = maxCounter;
    _count = 0;
  }

  activate() {
    _count = _count + 1;
    if (_count >= _countMax) {
      super.activate();
      _count = 0;
    }

    return this;
  }
}

main() {
  Stopwatch watch = new Stopwatch();
  watch.start();

  var n = 100000;
  var val = true;
  var toggle = new Toggle(val);

  for (var i = 0; i < n; i++) {
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
    val = toggle.activate().value;
  }

  print(toggle.value);

  val = true;
  var ntoggle = new NthToggle(val, 3);

  for (var i = 0; i < n; i++) {
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
    val = ntoggle.activate().value;
  }

  print(ntoggle.value);
  print("elapsed: ${watch.elapsedMilliseconds / 1000}");
}
