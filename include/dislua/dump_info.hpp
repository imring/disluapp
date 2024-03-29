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

#ifndef DISLUA_DUMP_INFO_H
#define DISLUA_DUMP_INFO_H

#include <complex>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "buffer.hpp"
#include "const.hpp"

#include "types/proto.hpp"

namespace dislua {
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
  explicit dump_info(const buffer &_buf = {}) : buf(_buf) {}

  /**
   * @brief Copy constructor.
   *
   * @param[in] rv Class with information about the script.
   */
  dump_info(const dump_info &rv) = default;

  virtual ~dump_info() = default;

  /**
   * @brief Read the information from the buffer.
   *
   * @exception std::runtime_error Parsing error.
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  virtual void read() {
    throw std::runtime_error{"Unknown compiler."};
  }
  /**
   * @brief Write the information to the buffer.
   *
   * @exception std::runtime_error Parsing error.
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  virtual void write() {
    throw std::runtime_error{"Unknown compiler."};
  }
  /// Get compiler (see dislua::compilers).
  virtual compilers compiler() const {
    return compilers::unknown;
  }

  /// Reset all informations.
  void reset() {
    version = 0;
    header = {};
    protos.clear();
    buf.reset();
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
  buffer buf;
};

template <typename T>
concept CompilerInterface = std::is_base_of_v<dump_info, T> && !std::is_same_v<T, dump_info>;
} // namespace dislua

#endif // DISLUA_DUMP_INFO_H