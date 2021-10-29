# v1.1.1 - 29.10.2021
- Fixed checking of header/prototype flags in LuaJIT.
- New unit test with check flags.
- Disable warning `unused functions` replaced with `[[maybe_unused]]`.
- The types `proto`/`varname` use [default comparisons](https://en.cppreference.com/w/cpp/language/default_comparisons).

# v1.1.0 - 26.09.2021
- Updated version format.
- New LuaJIT test.
- Updated flag formats (e.g. `KGC_CHILD` => `kgc::child`).
- `reinterpret_cast` replaced with `std::bit_cast` (C++20 required).

# v1.03 - 30.03.2021
- Added unit tests.
- Updated the documentation.
- Types from `dump_info` moved to a separate file and added `operator==` for `instruction`, `proto` and `varname`.

# v1.02 - 22.03.2021
- Fixed bug with undefined `BCM___`.
- Fixed bug with conflict between WinAPI's macro `min` and the function `std::numeric_limits<T>::min()`.

# v1.01 - 22.03.2021
- Predefined macro `CHECK_COMPILER` replaced with anonymous function `read_current_wo_exception`.
- Updated the documentation.
- Added the `.clang-format` file.
- Added functions to write lineinfo/upvalue names/varnames for LuaJIT.

# v1.00 - 21.03.2021
- Release.