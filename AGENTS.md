# Agent Guidance

## Model Selection

- Use the current model for fast iteration on planning, ticket writing, and
  small organizational updates.
- Recommend switching to a stronger model when the task becomes architecture-
  heavy, code-aware, or requires careful tradeoff analysis.
- For this bytecode work, suggest a stronger model before making final decisions
  on format, loader behavior, or compiler/runtime coupling.

## Workflow Notes

- Keep bytecode planning docs under `plan/`.
- Treat `plan/README.md` as the index.
- Treat `plan/bytecode-scope.md` as the v1 boundary.
- Use `plan/roadmap.md` for phases and `plan/tickets/` for detailed tasks.

## Scope Reminder

- v1 is single-file only.
- External module loading is out of scope.
- The artifact is version-locked.
- Obfuscation is out of scope.

## Build/Test Commands

**CRITICAL: `util/test.py` must be run from the repo root.** It resolves paths
relative to the working directory. Running it from `projects/make` will fail
with:

```
/Library/Developer/CommandLineTools/usr/bin/python3: can't open file
'.../projects/make/util/test.py': [Errno 2] No such file or directory
```

**CRITICAL: `wren_test.make` links against a prebuilt static library.** When
any source file under `src/vm/` (e.g., `wren_serialize.c`) changes, you must
rebuild the library with `wren.make` *before* building `wren_test`:

```
cd projects/make
make -f wren.make config=release_64bit
make -f wren_test.make config=release_64bit
```

Build the test binary from `projects/make`:

```
cd projects/make
make -f wren_test.make config=release_64bit
```

Run the test runner from the repo root:

```
cd ../..
python3 util/test.py api/bytecode_loader
```

Always build inside `projects/make` and run `util/test.py` from the repo root.
Do not invoke `util/test.py` from `projects/make` because it resolves paths
relative to the working directory.

`util/test.py` discovers and runs `.wren` files from `test/` and `example/`. It
parses comments like `// expect: <output>` and `// expect runtime error ...` in
the source file, then compares the binary's actual stdout/stderr/exit code to
those expectations.

Run the full suite:

```
python3 util/test.py
```

Run a subset by passing a path prefix relative to `test/`:

```
python3 util/test.py api/bytecode_loader
```

That matches the `--suite` argument and only runs files whose path starts with
the given prefix. Use this for targeted iteration instead of running every test.

### Debug / ASan builds

The debug test binary links against `libwren_d.a` and is produced with
`config=debug_64bit`. If the ASan debug link fails with an
`___asan_version_mismatch_check` symbol error, clean both the library and test
obj directories and rebuild from scratch:

```
cd projects/make
make -f wren.make config=debug_64bit clean
make -f wren_test.make config=debug_64bit clean
make -f wren.make config=debug_64bit
make -f wren_test.make config=debug_64bit
cd ../..
ASAN_OPTIONS=detect_leaks=0 python3 util/test.py --suffix _d
```
