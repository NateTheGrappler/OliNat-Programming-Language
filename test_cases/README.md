# Oli-Nat Test Suite

Golden-file tests for the Oli-Nat compiler and VM. Every `.oli` program sits next to
the exact output it must produce.

| File | Purpose |
|------|---------|
| `name.oli` | The program to run |
| `name.expected` | Exact stdout the program must print |
| `name.exitcode` | Optional. Expected exit code when it isn't 0 |
| `name.stdin` | Optional. Text fed to the program as input |

Exit codes: `0` success, `65` runtime error, `70` compile error.

## Running

```bash
cd cmake-build && cmake .. && make && cd ..
python3 tools/bin/run_tests.py            # run everything
python3 tools/bin/run_tests.py --update   # regenerate .expected files (review the git diff!)
```

## Layout

| Folder | What it covers |
|--------|----------------|
| `lexing/` | Tokens, literals, comments, identifiers that start with keywords |
| `types/` | Primitives, numeric promotion, division, type errors |
| `operators/` | Precedence, unary, comparison, short-circuiting, compound assignment |
| `variables_and_scope/` | Globals, block scope, shadowing |
| `control_flow/` | if/else chains, while, for, condition type errors |
| `functions/` | Parameters, return types, recursion, missing returns, stack overflow |
| `closures/` | Upvalue capture, sharing, mutation, loops, nesting |
| `arrays/` | Element types, indexing, reference semantics, bounds errors |
| `strings/` | Concatenation, interning, every `Strings` library function |
| `classes/` | Fields, defaults, methods, `this`, constructors |
| `inheritance/` | Inherited fields and methods, overriding, `pullf` parent calls |
| `stdlib/` | io, math, types, utils, fileIO, random, chronos, imports |
| `gc/` | Garbage collector stress tests: values must survive heavy allocation |
| `programs/` | End-to-end programs: FizzBuzz, sorting, primes, a bank, tic-tac-toe |
| `regressions/` | Tests for bugs that have been fixed, so they can't come back (created as bugs in `known_bugs/` get fixed) |

Files starting with `error_` must fail to compile (exit 70). Files starting with
`runtime_` must fail while running (exit 65), and any output printed before the error is
still checked.

## Adding a test

1. Write a small `.oli` file that tests one thing, with a name that says what it tests.
2. Write the `.expected` file **by hand**, from what the language *should* do.
3. Run the suite. If it fails, it's either a mistake in the test or a bug in the VM.
4. When you fix a bug, add the smallest program that reproduced it to `regressions/`.

PLEASE DO NOT ADD IN BUGGED TESTS