# White-Box Testing Lab: CFGs and Prime Path Coverage

## Learning goals

By the end of this lab, students should be able to:

1. Convert a simple C program into a Control Flow Graph.
2. Read the graph in both textual and graphical form.
3. Compute prime paths from the CFG.
4. Construct complete test paths that cover all prime paths.
5. Reason about how loop structure affects path coverage.

## Provided code

- `src/parser.cpp`: turns a simple C program into an abstract syntax tree.
- `src/cfg.cpp`: builds a CFG with numbered nodes.
- `src/analysis.cpp`: reference implementation for prime path enumeration and test-path selection.
- `samples/*.c`: example input programs.

## Suggested lab flow

1. Build the project with CMake.
2. Run `cfg_lab` on `samples/prime_path_example.c`.
3. Inspect the textual CFG and the rendered DOT/SVG graph.
4. Identify the node IDs corresponding to decisions, loop headers, and returns.
5. Run the tool with `--analysis` and compare the reported prime paths with a manual derivation.
6. Ask students to re-implement or extend the logic in `src/analysis.cpp`.

## Suggested student tasks

1. Trace the source program and explain why each CFG edge exists.
2. Manually list all simple paths and determine which ones are prime.
3. Check whether the automatically selected test paths are minimal.
4. Modify one sample program by adding an extra branch inside a loop, then observe how the prime-path set changes.
5. Extend the parser or CFG builder to support one extra C construct, such as `switch`.

## Example commands

Build:

```bash
cmake -S . -B build
cmake --build build
```

Text CFG:

```bash
./build/cfg_lab samples/prime_path_example.c
```

Rendered CFG:

```bash
./build/cfg_lab --dot build/example.dot --render build/example.svg samples/prime_path_example.c
```

Prime-path analysis:

```bash
./build/cfg_lab --analysis --max-visits 3 samples/prime_path_example.c
```

## Discussion prompts

1. Why does a prime path stop being prime once it becomes a proper subpath of a longer simple path?
2. Why is generating a minimum number of test paths harder than just listing the prime paths?
3. Why can a prime path that starts inside a cycle require more than two visits to some CFG nodes in a complete test path?
