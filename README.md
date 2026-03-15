# ***ParaCL***
>*homework for C++ base course 25/26*

An educational programming C-like language, implemented as an interpreter. The project includes a lexical analyzer (Flex), a parser (Bison), an abstract syntax tree (AST), and an interpreter that traverses the AST.

### Execution pipeline
`Parse -> SemanticChecker -> Interpreter`

### Features
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparisons: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Logical operators: `&&`, `||`, `!`, `^`
- Unary operators: `+`, `-`, `!`
- Variable assignment
- Input: `?` reads an integer from keyboard
- Output: `print <int64_t>`
- Cycles: `if`/`else`, `while`, `for`
- Scope blocks `{ ... }`
- Comments `// ...`

### Requirements
- C++20
- CMake 3.21+
- Flex 2.6+
- Bison 3.7+
- Graphviz
- Python 3 (for e2e runner)

### Installation:
```sh
git clone git@github.com:rhagewannakiss/ParaCL.git
cd ParaCL
```

### Build:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
Or with graphviz visualization:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DGRAPHVIZ=ON
```
Then:
```sh
cmake --build build
```
Binary:
```sh
./build/bin/paracl-cli
```

Optional developer targets (if tools are installed):
```sh
cmake --build build --target check-format
cmake --build build --target format
cmake --build build --target cppcheck
```

### Usage:
```sh
./build/bin/paracl-cli examples/<input_file>
```
With stdin:
```sh
./build/bin/paracl-cli examples/simple_input.pcl < test/e2e/valid_progs/simple_input.in
```

### Run Google-tests:
```sh
cd build
ctest --output-on-failure
```
Alternative from project root:
```sh
ctest --test-dir build --output-on-failure
```

### Run e2e tests:
```sh
python3 test/e2e/run_e2e.py
```

## Authors:
- *Ostafeichuk Roman, B01-401 DREC*
- *Makarskaya Alexandra, B01-401 DREC*
