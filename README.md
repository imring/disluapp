# DisLua
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

DisLua is a header-only library that allows you to parse and rewrite the bytecode of compiled Lua scripts. At the moment, the library supports the [LuaJIT](http://luajit.org/) compiler and a parser for [luac](https://www.lua.org/) v5.4 is being developed (branch [luac](https://github.com/imring/disluapp/tree/luac)).

## Projects using this library
- [Luad](https://github.com/imring/luad) - Disassembler for compiled Lua scripts.

## Documentation
You can build Doxygen documentation using CMake specifying the option `-DBUILD_DOC=ON`, or manually using the command `doxygen`.

## Use in projects
Install the library using CMake:
```bash
$ mkdir build && cd build
$ cmake ..
$ sudo cmake --build . --target INSTALL
```

Add `dislua` to your CMake-project by using:
```cmake
find_package(dislua REQUIRED)

target_link_libraries(yourproject PRIVATE dislua)
```