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

#ifndef DISLUA_TYPES_TABLE_H
#define DISLUA_TYPES_TABLE_H

#include <unordered_map>
#include <variant>
#include <string>

#include "../const.hpp"

namespace dislua {
/**
 * @brief Type with variation of key/value type table.
 *
 * Variants in LuaJIT:
 * - std::nullptr_t: KTAB_NIL (nil);
 * - bool: KTAB_FALSE/KTAB_TRUE (true/false);
 * - dislua::leb128: KTAB_INT (123);
 * - double: KTAB_NUM (123.456);
 * - std::string: KTAB_STR ("Hello, World!").
 */
using table_val_t = std::variant<std::nullptr_t, bool, leb128, double, std::string>;

/**
 * @brief Table type.
 *
 * Example for LuaJIT:
 * @code
 * dislua::table_t t = {
 *   { 123, 123.456 }
 * };
 * t[456] = nullptr;
 * t[true] = "Hello, World!";
 * @endcode
 */
using table_t = std::unordered_map<table_val_t, table_val_t>;
} // namespace dislua

#endif // DISLUA_TYPES_TABLE_H