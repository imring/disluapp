// Project: disluapp
// URL: https://github.com/imring/disluapp/

// MIT License

// Copyright (c) 2020-2022 Vitaliy Vorobets

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

#ifndef DISLUA_TYPES_INSTRUCTION_H
#define DISLUA_TYPES_INSTRUCTION_H

#include "../const.hpp"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

namespace dislua {
/**
 * @brief Instruction type.
 *
 * Format in LuaJIT (4 bytes):
 * @code
 * +----+----+----+----+
 * | B  | C  | A  | OP | Format ABC
 * +----+----+----+----+
 * |    D    | A  | OP | Format AD
 * +---------+----+----+
 * MSB               LSB
 * @endcode
 */
struct instruction {
  instruction() : opcode(0), a(0), d(0) {}
  instruction(uchar opcode, uchar a, ushort d) : opcode(opcode), a(a), d(d) {}
  instruction(uchar opcode, uchar a, uchar b, uchar c) : opcode(opcode), a(a), b(b), c(c) {}

  uchar opcode;

  // fields
  uchar a;
  union {
    struct {
      uchar c, b;
    };
    ushort d;
  };

  friend bool operator==(const dislua::instruction &left, const dislua::instruction &right) {
    return left.opcode == right.opcode && left.a == right.a && left.d == right.d;
  }
};
} // namespace dislua

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // DISLUA_TYPES_INSTRUCTION_H