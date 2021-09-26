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

#ifndef DISLUA_TYPES_KGC_H
#define DISLUA_TYPES_KGC_H

#include <complex>
#include <string>
#include <variant>

#include "../const.hpp"
#include "table.hpp"

namespace dislua {
struct proto;

/**
 * @brief Variation type for constant GC variables.
 *
 * Variants in LuaJIT:
 * - dislua::proto: KGC_CHILD;
 * - dislua::table_t: KGC_TAB ({1, 2, ["test"] = false});
 * - long long: KGC_I64 (-123);
 * - unsigned long long: KGC_U64 (123);
 * - std::complex<double>: KGC_COMPLEX (0+1i);
 * - std::string: KGC_STR ("Hello, World!").
 */
using kgc_t = std::variant<proto, table_t, long long, unsigned long long, std::complex<double>, std::string>;
} // namespace dislua

#endif // DISLUA_TYPES_KGC_H