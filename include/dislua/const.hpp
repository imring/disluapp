// Project: disluapp
// URL: https://github.com/imring/disluapp/

// MIT License

// Copyright (c) 2020-2021 Vitaliy Vorobets

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef DISLUA_CONST_H
#define DISLUA_CONST_H

#include <cmath>
#include <limits>
#include <type_traits>

/**
 * @brief disluapp version.
 * Format: MAJOR.MINOR.PATCH
 */
#define DISLUA_VERSION 111L

namespace dislua {
using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

/// Unsigned type [LEB128](https://en.wikipedia.org/wiki/LEB128).
using uleb128 = uint;
/// Signed type [LEB128](https://en.wikipedia.org/wiki/LEB128).
using leb128 = std::make_signed_t<uleb128>;

/// List of compiler versions supported by this library.
enum class compilers {
  /// Unknown compiler.
  unknown = -1,
  /// [LuaJIT](https://luajit.org/) (the library supports v1 & v2).
  luajit
};
} // namespace dislua

#endif // DISLUA_CONST_H