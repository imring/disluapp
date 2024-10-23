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

#ifndef DISLUA_TYPES_VARNAME_H
#define DISLUA_TYPES_VARNAME_H

#include <string>
#include <string_view>

#include "../const.hpp"

namespace dislua {
/**
 * @brief Variables name (debug information, for LuaJIT).
 */
struct varname {
  varname() = default;
  varname(uchar type, size_t start, size_t end, std::string_view name = "")
      : type(type), name(name), start(start), end(end){}

  uchar type = 0;
  std::string name;
  size_t start = 0, end = 0;

  friend bool operator==(const dislua::varname &left, const dislua::varname &right) = default;
};
} // namespace dislua

#endif // DISLUA_TYPES_VARNAME_H