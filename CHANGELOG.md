# v1.02 - 22.03.2021
- Fixed bug with undefined `BCM___`.
- Fixed bug with conflict between WinAPI's macro `min` and the function `std::numeric_limits<T>::min()`.

# v1.01 - 22.03.2021
- Predefined macros `CHECK_COMPILER` was replaced with anonymous function `read_current_wo_exception`.
- Updated the documentation.
- Added the `.clang-format` file.
- Added functions to write lineinfo/upvalue names/varnames for LuaJIT.

# v1.00 - 21.03.2021
- Release.