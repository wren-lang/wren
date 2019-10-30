## 0.2.0

0.2.0 spans a pretty wide time period with [around 290 commits](https://github.com/wren-lang/wren/compare/0.1.0...master).
This includes many bug fixes, improvements, clarity in the
code and documentation and so on. There's too many to explicitly list.
Below is the obvious user facing stuff that was easy to spot in the history.

Most noteworthy is that 'relative imports' are a slightly breaking change,
but help pave the way forward toward a consistency for modules.

### Language/VM

- `import` was made smarter, differentiating relative from logical
- `Fiber` can now accept a value from the first `call`/`transfer`
- Added `String.trim`, `String.trimEnd`, `String.trimStart` variants
- Added `String.split`, `String.replace`, `String.fromByte`
- Added `String.indexOf(needle, startIndex)`
- Added `Sequence.take` and `Sequence.skip`
- Added `List.filled(count, value)`
- Added `Num.pow`, `Num.log`, `Num.round`
- Added `Num.largest`, `Num.smallest`
- Added `Map` iteration (`MapEntry`)

#### C API

- Added `wren.hpp` for use in c++
- Added void* user data to `WrenVM`
- Allow hosts with no module loader to still load optional modules.
- Added `wrenAbortFiber`

### CLI
Please note that beyond 0.2.0 the CLI will have it's own changelog.
This list is not exhaustive. For a fuller history see the commit log above.

- Add path module
- Add `--version`
- Add REPL written in Wren
- Add Stdin.isTerminal
- Added Platform class
- Rename `process` module to `os`

## 0.1.0

First declared version. Everything is new!
