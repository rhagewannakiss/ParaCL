# ***ParaCL***
>*homework for C++ base course 25/26*

An educational programming C-like language, implemented as an interpreter. The project includes a lexical analyzer (Flex), a parser (Bison), an abstract syntax tree (AST), and an interpreter that traverses the AST.

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

### Usage:
```sh
./build/bin/paracl-cli examples/<input_file>
```

### Run Google-tests:
```sh
cd build
ctest --output-on-failure
```

## Authors:
- *Ostafeichuk Roman, B01-401 DREC*
- *Makarskaya Alexandra, B01-401 DREC*
