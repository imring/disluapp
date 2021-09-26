# DisLua
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/imring/disluapp)](https://github.com/imring/disluapp/releases/latest)

DisLua is a header-only library that allows you to parse and rewrite the bytecode of compiled Lua scripts. At the moment, the library supports the [LuaJIT](http://luajit.org/) compiler and a parser for [luac](https://www.lua.org/) v5.4 is being developed (branch [luac](https://github.com/imring/disluapp/tree/luac)).

## Projects using this library
- [Luad](https://github.com/imring/luad) - Disassembler for compiled Lua scripts.

## Documentation
You can build [Doxygen](https://www.doxygen.nl/index.html) documentation:
- Using CMake specifying the `DISLUA_DOCS` option:
```bash
$ cmake .. -DDISLUA_DOCS=ON -B build
$ cd build
$ cmake --build . --target doc
```
- Manually using the command `doxygen`.

## Testing
You can test the library using CTest specifying the `DISLUA_TESTS` option (C++20 required):
```bash
$ cmake .. -DDISLUA_TESTS=ON -B build
$ cd build
$ cmake --build .
$ ctest
```

## Use in projects
Install the library using CMake (`DISLUA_INSTALL` option is enabled by default):
```bash
$ cmake .. -B build
$ cd build
$ cmake --build . --target install
```

Add `dislua` to your CMake-project by using:
```cmake
find_package(dislua REQUIRED)

target_link_libraries(yourproject PRIVATE dislua)
```