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

#ifndef DISLUA_DUMP_INFO_H
#define DISLUA_DUMP_INFO_H

#include <complex>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "buffer.hpp"
#include "const.hpp"

namespace dislua {
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

/**
 * @brief Class with all information about the compiled Lua script.
 * 
 * This class is the backbone of parsers.
 */
class dump_info {
public:
  /**
   * @brief Constructor with a class buffer.
   *
   * @param[in] _buf Buffer.
   */
  dump_info(buffer &_buf) : buf(std::make_unique<buffer>(_buf)) {}

  /**
   * @brief Constructor with a class dump_info.
   *
   * @param[in] rv Class with information about the script.
   */
  dump_info(const dump_info &rv)
      : header(rv.header), version(rv.version), protos(rv.protos),
        buf(std::make_unique<buffer>(*rv.buf)) {}

  virtual ~dump_info() {}

  struct proto;

  //// Type with variation of key/value type table.
  using table_val_t =
      std::variant<std::nullptr_t, bool, leb128, double, std::string>;

  /// Table type.
  using table_t = std::unordered_map<table_val_t, table_val_t>;

  /// Variation type for constant GC variables.
  using kgc_t = std::variant<proto, table_t, long long, unsigned long long,
                             std::complex<double>, std::string>;

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
    uchar opcode = 0;

    // fields
    uchar a = 0;
    union {
      struct {
        uchar c, b;
      };
      ushort d = 0;
    };
  };

  /// @brief Variables name (debug information, for LuaJIT).
  struct varname {
    uchar type = 0;
    std::string name;
    size_t start = 0, end = 0;
  };

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
  };

  /// Read the information from the buffer.
  virtual void read() { throw std::runtime_error("Unknown compiler."); }
  /// Write the information to the buffer.
  virtual void write() { throw std::runtime_error("Unknown compiler."); }
  /// Get compiler.
  virtual compilers compiler() { return DISLUA_UNKNOWN; }

  /// Reset all informations.
  void reset() {
    version = 0;

    header.flags = 0;
    header.debug_name.clear();

    protos.clear();
    buf->reset();
  }

  /// Header info.
  struct {
    uint flags = 0;
    std::string debug_name;
  } header;

  /// Compiled Lua script version.
  uint version = 0;

  /// Container with prototypes.
  std::vector<proto> protos;
  /// Main buffer.
  std::unique_ptr<buffer> buf;
};

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
} // namespace dislua

#endif // DISLUA_DUMP_INFO_H