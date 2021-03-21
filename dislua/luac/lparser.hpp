// dislua

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

#ifndef DISLUA_LUA_PARSER_H
#define DISLUA_LUA_PARSER_H

#include "../const.hpp"
#include "../dump_info.hpp"
#include "lconst.hpp"

namespace dislua::luac {
class parser : public dump_info {
  int64_t read_integer() {
    if (sizes.sint == sizeof(int32_t))
      return buf->read<int32_t>();
    else if (sizes.sint == sizeof(int64_t))
      return buf->read<int64_t>();
    else
      throw std::runtime_error("Luac: Unknown integer type.");
  }

  double read_number() {
    if (sizes.snum == sizeof(float))
      return buf->read<float>();
    else if (sizes.snum == sizeof(double))
      return buf->read<double>();
    else
      throw std::runtime_error("Luac: Unknown number type.");
  }

  void read_header() {
    constexpr size_t shead = sizeof(header::SIGNATURE) - 1;
    char head[shead];
    buf->read(head, shead);
    if (strncmp(header::SIGNATURE, head, shead))
      throw std::runtime_error("Luac: Invalid header.");

    version = static_cast<uint>(buf->read());
    if (version != 0x54)
      throw std::runtime_error("Luac: Unknown version."); // Only Lua 5.4.
    format = static_cast<uint>(buf->read());
    if (format != 0)
      throw std::runtime_error("Luac: Unknown format.");

    constexpr size_t sdata = sizeof(header::DATA) - 1;
    char data[sdata];
    buf->read(data, sdata);
    if (strncmp(header::DATA, data, sdata))
      throw std::runtime_error("Luac: Corrupted chunk.");

    sizes.sins = static_cast<uint>(buf->read());
    if (sizes.sins != 4)
      throw std::runtime_error("Luac: Invalid instruction size.");
    sizes.sint = static_cast<uint>(buf->read());
    if (sizes.sins != sizeof(int32_t) && sizes.sins != sizeof(int64_t))
      throw std::runtime_error("Luac: Invalid integer size.");
    sizes.snum = static_cast<uint>(buf->read());
    if (sizes.snum != sizeof(float) && sizes.snum != sizeof(double))
      throw std::runtime_error("Luac: Invalid number size.");

    if (read_integer() != header::INT ||
        !almost_equal(read_number(), header::NUM, 2))
      throw std::runtime_error("Luac: Number(s) format mismatch.");
  }

public:
  parser(buffer &_buf) : dump_info(_buf) {}
  parser(const dump_info &rv) : dump_info(rv) {}
  ~parser() {}

  void read() { read_header(); }
  void write() {}
  compilers compiler() { return DISLUA_LUAC; }
};
} // namespace dislua::luac

#endif // DISLUA_LUA_PARSER_H