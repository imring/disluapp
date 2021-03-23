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

#pragma push_macro("min")
#undef min

namespace dislua {
using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;

/// Unsigned type [LEB128](https://en.wikipedia.org/wiki/LEB128).
using uleb128 = uint;
/// Signed type [LEB128](https://en.wikipedia.org/wiki/LEB128).
using leb128 = std::make_signed_t<uleb128>;

// Version format: major * 100 + minor.
constexpr uint version = 102;

/// List of compiler versions supported by this library.
enum compilers {
  /// Unknown compiler.
  DISLUA_UNKNOWN = -1,
  /// [LuaJIT](https://luajit.org/) (the library supports v1 & v2).
  DISLUA_LUAJIT
};

namespace {
// https://en.cppreference.com/w/cpp/utility/variant/visit
// https://stackoverflow.com/questions/46604950/what-does-operator-mean-in-code-of-c
template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// https://habr.com/ru/post/106294/#comment_3341606
/// This template functions should be used when we perform cast from one pointer
/// type to another It's safer than using reiterpret_cast
///
/// It doesn't allow to do such things like:
/// int i = 10;
/// A *a = pointer_cast<A*>(i);
/// Only pointer could be used in this function.
template <typename result, typename source>
static inline result pointer_cast(source *v) {
  return static_cast<result>(static_cast<void *>(v));
}

template <typename result, typename source>
static inline result pointer_cast(const source *v) {
  return static_cast<result>(static_cast<const void *>(v));
}

// https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
template <class T>
static typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp) {
  // the machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  return std::fabs(x-y) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * ulp
      // unless the result is subnormal
      || std::fabs(x-y) < std::numeric_limits<T>::min();
}
} // namespace
} // namespace dislua

#pragma pop_macro("min")

#endif // DISLUA_CONST_H