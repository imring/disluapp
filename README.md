# DisLua
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Release](https://img.shields.io/github/v/release/imring/disluapp)](https://github.com/imring/disluapp/releases/latest)

DisLua is a header-only library that allows you to parse and rewrite the bytecode of compiled Lua scripts. At the moment, the library supports the [LuaJIT](http://luajit.org/) compiler and a parser for [luac](https://www.lua.org/) v5.4 is being developed (branch [luac](https://github.com/imring/disluapp/tree/luac)).

## Projects using this library
- [Luad](https://github.com/imring/luad) - Disassembler for compiled Lua scripts.

## Documentation
You can build Doxygen documentation:
- Using CMake specifying the option `-DBUILD_DOC=ON`:
```bash
$ mkdir build && cd build
$ cmake .. -DBUILD_DOC=ON
$ cmake --build . --target doc
```
- Manually using the command `doxygen`.

## Testing
You can test the library using CTest specifying the option `-DBUILD_TEST=ON`:
```bash
$ mkdir build && cd build
$ cmake .. -DBUILD_TEST=ON
$ cmake --build .
$ cd tests
$ ctest
```

## Use in projects
Install the library using CMake:
```bash
$ mkdir build && cd build
$ cmake ..
$ sudo cmake --build . --target install
```

Add `dislua` to your CMake-project by using:
```cmake
find_package(dislua REQUIRED)

target_link_libraries(yourproject PRIVATE dislua)
```