# CFG Lab Package

This repository contains a compact lab package for teaching white-box testing with control-flow graphs and Prime Path Coverage.

## What is included

- `cfg_lab`: a C++ command-line tool that parses a documented subset of C, builds a numbered CFG, exports Graphviz DOT, and computes prime paths plus test paths.
- `prime_path_demo`: a smaller hard-coded example that demonstrates the path algorithms without requiring parsing.
- `samples/`: example C programs for the lab.
- `docs/LAB_HANDOUT.md`: a student-facing lab outline.

## Supported C subset

The parser is intentionally small and is designed for teaching examples rather than full C compilation. It supports:

- Function definitions.
- Blocks `{ ... }`.
- Expression and declaration statements ending in `;`.
- `if` / `else`.
- `while`, `do ... while`, and `for`.
- `return`, `break`, and `continue`.
- Single-line and block comments.
- Preprocessor lines such as `#include <stdio.h>` are ignored.

Not supported: `switch`, `goto`, macros with embedded control flow, function pointers, and full C declarator syntax.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

Generate a numbered CFG in text form:

```bash
./build/cfg_lab samples/prime_path_example.c
```

Generate DOT and render SVG:

```bash
./build/cfg_lab --dot build/prime_path_example.dot --render build/prime_path_example.svg samples/prime_path_example.c
```

Compute prime paths and the selected covering test paths:

```bash
./build/cfg_lab --analysis --max-visits 3 samples/prime_path_example.c
```

Run the smaller hard-coded demo:

```bash
./build/prime_path_demo
```

## Output conventions

- CFG nodes are assigned integer IDs starting from `1`.
- Paths are printed in the form `1-2-3-4`.
- `ENTRY` and `EXIT` nodes are included explicitly so complete test paths are unambiguous.

## Notes on minimum test paths

The sample implementation computes:

1. All prime paths in the CFG.
2. All complete entry-to-exit paths under a configurable node-visit bound.
3. An exact minimum set cover over those candidate complete paths.

This is suitable for lab-scale graphs. The default `--max-visits` value is `3`, which is enough for many structured loop examples. Increase it further for loop-heavy CFGs.
