## 0.4.0

### Language
- Add `continue` keyword
- Add `as`: `import "..." for Name as OtherName`
- Add Support positive sign in scientific notation
- Add Fiber.try(value) to complement Fiber.call(value)
- Allow `.` to be on a different line (for fluent/builder APIs)

### Modules
- Random: Random.sample optimizations
- List:
  - add `list.sort()` and `list.sort {|a, b| ... }` (quicksort)
  - add `list.swap(index0, index1)` for swapping elements within a list
  - add `list.indexOf(value)` for finding values in a list
- Num:
  - add `Num.tau`
  - add `Num.nan`
  - add `Num.infinity`
  - add `min(other)`
  - add `max(other)`
  - add `clamp(min, max)`
  - add `exp`
  - add `log2`

### Fixes
- Fix stack corruption related to `Fn` calls
- Fix a byte offset bug in CODE_IMPORT_VARIABLE
- Fix some stack corruptions related to multiple wrenInterpret calls
- Fixed crash when GC collects module during import
- Fix `Bool`, `Num` and `Null` allowing subclassing, which is invalid

### API
- BREAKING: Add `userData` to `wrenReallocateFn`
- BREAKING: Add `WrenLoadModuleResult` which has a `onComplete` callback, allowing freeing module strings
- Add `wrenHasVariable` and `wrenHasModule` queries, for use with `wrenGetVariable`
- Add `wrenSetListElement` to complement `wrenGetListElement`, and allow negative index for both
- Add Map functions to API
  - wrenSetSlotNewMap
  - wrenGetMapCount
  - wrenGetMapContainsKey
  - wrenGetMapValue
  - wrenSetMapValue
  - wrenRemoveMapValue

### Other
- build; add util/generate_docs.py for regenerating project files
- vm; Allow computed goto when using clang on Windows
- vm; WREN_MAX_TEMP_ROOTS default is 8 (instead of 5)
- vm; GC debug times are printed in milliseconds, not seconds

## 0.3.0

0.3.0 is a fairly specific release, aimed at fixing build issues across platforms,
streamlining the process for new users and making embedding easier.
This is a stepping stone for working on language features and improving the VM,
hacking on the docs and the VM is simpler than ever!

Builds now work out of the box on all primary platforms.
Previously there was issues on Windows and other platforms due to unix-ey workflows being the default.

All the python scripts have also been fixed and updated (to python 3), and work consistently
across all platforms out of the box too (including the tests, benchmarks, metrics etc).
Like before, there was some things that didn't hold up on Windows or Mac. Fixed!

A lot of work has been done to also clarify the distinction between the CLI project and the VM,
as well as [move the CLI to its own repo](https://github.com/wren-lang/wren-cli/)!
This removes a lot of code that wasn't being used, and also been clarified the project structure.

Docs have also had a clean up, and a new page to try Wren directly on the doc page was added.

### Language/VM

- CLI moved to own repo
- Use premake for project generation, see projects/
- Fix builds across platforms. "Just works" on all primary platforms.
- Fix amalgamated script generator and amalgamated build
- Fix unicode parsing and other issues in all python scripts
- All python scripts are python3 now, and run on all platforms correctly
- Test runner isolated and unified for VM tests
- Remove SASS and Pygments requirements from docs, just python now
- Updated docs to clarify VM/CLI split
- Added Try page for running wren code in the docs

## 0.2.0

0.2.0 spans a pretty wide time period with [around 290 commits](https://github.com/wren-lang/wren/compare/0.1.0...main).
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
