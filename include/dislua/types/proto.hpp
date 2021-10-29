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

#ifndef DISLUA_TYPES_PROTO_H
#define DISLUA_TYPES_PROTO_H

#include <vector>
#include <string>

#include "../const.hpp"
#include "kgc.hpp"
#include "varname.hpp"
#include "instruction.hpp"

namespace dislua {
/// Prototype information.
struct proto {
  uchar flags = 0, numparams = 0, framesize = 0;
  uleb128 firstline = 0, numline = 0;

  std::vector<instruction> ins;
  std::vector<ushort> uv;
  std::vector<kgc_t> kgc;
  std::vector<double> knum;

  std::vector<uint> lineinfo;
  std::vector<std::string> uv_names;
  std::vector<varname> varnames;

  friend bool operator==(const dislua::proto &left, const dislua::proto &right) = default;
};
} // namespace dislua

#endif // DISLUA_TYPES_PROTO_H