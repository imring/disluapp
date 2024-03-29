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

#ifndef DISLUA_H
#define DISLUA_H

#include <memory>
#include <type_traits>

#include "buffer.hpp"
#include "const.hpp"
#include "detail.hpp"
#include "dump_info.hpp"

#include "lj/ljparser.hpp"

/**
 * @brief DisLua library namespace.
 *
 * DisLua is a library that allows you to parse and rewrite the bytecode of
 * compiled Lua scripts.
 */
namespace dislua {
/**
 * @brief Parse the buffer using the specific parser.
 *
 * @code{.cpp}
 * dislua::buffer buf{...};
 * try {
 *   auto info = dislua::read_current<dislua::lj::parser>(buf);
 * } catch (const std::exception &e) {
 *   std::cerr << e.what() << '\n';
 * }
 * @endcode
 *
 * @tparam T Parser type (e.g. dislua::lj::parser).
 * @param[in] buf Buffer with compiled lua script.
 * @return std::unique_ptr<dump_info> Smart pointer to compiled lua script
 * information.
 *
 * @warning T-type must be child of dislua::dump_info.
 *
 * @note If you need to parse with all the parsers that are available in the
 * DisLua library, then use dislua::read_all.
 *
 * @exception std::runtime_error Parsing error.
 * @exception std::out_of_range An error occurred while exiting the container.
 */
template <CompilerInterface T>
std::unique_ptr<dump_info> read_current(const buffer &buf) {
  std::unique_ptr<dump_info> check = std::make_unique<T>(buf);
  check->read();
  return check;
}

namespace {
template <CompilerInterface T>
std::unique_ptr<dump_info> read_current_wo_exception(const buffer &buf) try {
  return read_current<T>(buf);
} catch (...) {
  return {};
}
} // namespace

/**
 * @brief Parse the buffer with all parsers available in the DisLua library.
 *
 * @code{.cpp}
 * dislua::buffer buf{...};
 * auto info = dislua::read_all(buf);
 * if (!info) {
 *   std::cerr << "Unknown compiler of lua script.\n";
 *   return 1;
 * }
 * @endcode
 *
 * @param[in] buf Buffer with compiled lua script.
 * @return std::unique_ptr<dump_info> Smart pointer to compiled lua script
 * information.
 */
[[maybe_unused]] inline std::unique_ptr<dump_info> read_all(const buffer &buf) {
  std::unique_ptr<dump_info> in = read_current_wo_exception<lj::parser>(buf);
  return in;
}
} // namespace dislua

#endif // DISLUA_H