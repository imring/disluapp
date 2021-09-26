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

#ifndef DISLUA_BUFFER_H
#define DISLUA_BUFFER_H

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "const.hpp"

namespace dislua {
/// Class buffer for parsing/writing Lua bytecode.
class buffer {
  std::vector<uchar> d;

public:
  buffer() = default;

  /**
   * @brief Constructs with two iterators.
   *
   * @code{.cpp}
   * // input iterators
   * std::ifstream luac("./file.luac", std::ios::binary);
   * dislua::buffer buf_input((std::istreambuf_iterator<char>(luac)),
   *                          std::istreambuf_iterator<char>());
   *
   * // forward iterators
   * std::vector<unsigned char> a = {1, 2, 3};
   * dislua::buffer buf_forward(a.begin(), a.end());
   * @endcode
   *
   * @tparam It Iterator type.
   * @param[in] first,last Range of elements.
   */
  template <typename It>
  buffer(It first, It last) {
    write(first, last);
    reset_indices();
  }

  /**
   * @brief Constructs with an initialization list.
   *
   * @code{.cpp}
   * dislua::buffer buf{1, 2, 3};
   * @endcode
   *
   * @tparam T Element type.
   * @param[in] arr Initialization list.
   */
  template <typename T>
  buffer(std::initializer_list<T> arr) : buffer(arr.begin(), arr.end()) {}

  /**
   * @brief Constructs with a class buffer.
   *
   * @param[in] buf Class buffer.
   */
  buffer(const buffer &buf) : buffer(buf.d.begin(), buf.d.end()) {
    reset_indices();
  }

  /**
   * @brief Reads values from the buffer and writes to the T-type array.
   *
   * @code{.cpp}
   * dislua::buffer buf{ 0x10, 0x80, 0xD, 0x2 };
   * dislua::uchar result[3] = { 0 };
   * buf.read(result, 1, false);
   * buf.read(&result[1], 2);
   *
   * std::cout << std::hex << std::uppercase;
   * for (dislua::uchar val: result)
   *   std::cout << +val << ' ';
   *
   * // output: 10 10 80
   * @endcode
   *
   * @tparam T Value type.
   * @param[out] arr Array.
   * @param[in] number Amount of elements.
   * @param[in] next Move to the next index to be read.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  template <typename T>
  void read(T *arr, size_t number, bool next = true) {
    size_t s = sizeof(T) * number;
    if (iread + s - 1 >= size())
      throw std::out_of_range("Read index better than container size.");

    uchar *data = d.data();
    std::memcpy(arr, data + iread, s);
    if (next)
      iread += s;
  }

  /**
   * @brief Reads values from the buffer and writes to the container using two
   * iterators.
   *
   * @code{.cpp}
   * dislua::buffer buf{ 0x10, 0x80, 0xD, 0x2 };
   * dislua::uchar result[3] = { 0 };
   * buf.read(std::begin(result), std::begin(result) + 1, false);
   * buf.read(std::begin(result) + 1, std::end(result));
   *
   * std::cout << std::hex << std::uppercase;
   * for (dislua::uchar val: result)
   *   std::cout << +val << ' ';
   * // output: 10 10 80
   * @endcode
   *
   * @tparam It Iterator type.
   * @param[out] first,last Range of elements.
   * @param[in] next Move to the next index to be read.
   *
   * @warning Size of value type of the iterator must be equal to size of
   * unsigned char.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  template <typename It>
  void read(It first, It last, bool next = true) {
    using T = typename std::iterator_traits<It>::value_type;
    // static_assert(sizeof(T) == sizeof(uchar),
    //               "Size of type not equals size of byte type.");

    auto s = static_cast<size_t>(std::distance(first, last));
    if (iread + s - 1 >= size())
      throw std::out_of_range("Read index better than container size.");

    auto start = d.begin() + static_cast<std::ptrdiff_t>(iread);
    std::copy(start, start + static_cast<std::ptrdiff_t>(s), first);
    if (next)
      iread += s;
  }

  /**
   * @brief Writes values from the container to the buffer using two input iterators.
   *
   * @code{.cpp}
   * std::ifstream luac(...);
   * dislua::buffer buf;
   * buf.write((std::istreambuf_iterator<char>(luac)),
   *           std::istreambuf_iterator<char>());
   * @endcode
   *
   * @tparam It Iterator type.
   * @param[in] first,last Range of elements.
   *
   * @warning Size of value type of the iterator must be equal to size of
   * unsigned char.
   */
  template <typename It>
  void write(It first, It last, std::input_iterator_tag) {
    using T = typename std::iterator_traits<It>::value_type;
    // static_assert(sizeof(T) == sizeof(uchar),
    //               "Size of type not equals size of byte type.");

    std::vector<uchar> v(first, last);
    write(v.begin(), v.end());

    /*size_t os = size();
    std::copy(first, last,
              std::inserter(d, d.begin() + std::ptrdiff_t(iwrite)));

    size_t offset = os - iwrite;
    size_t ds = size() - os;
    if (offset != 0) {
      if (ds < offset) {
        decltype(d)::iterator start = d.begin() + std::ptrdiff_t(iwrite + ds);
        std::copy(start + std::ptrdiff_t(ds),
                  start + std::ptrdiff_t(ds + (os - ds)), start);
        d.erase(d.end() - std::ptrdiff_t(ds), d.end());
      } else
        d.erase(d.end() - std::ptrdiff_t(offset), d.end());
    }
    iwrite += ds;*/
  }

  /**
   * @brief Writes values from the container to the buffer using two forward
   * iterators.
   *
   * @code{.cpp}
   * std::vector<dislua::uchar> v{1,2,3};
   * dislua::buffer buf;
   * buf.write(v.begin(), v.end());
   * @endcode
   *
   * @tparam It Iterator type.
   * @param[in] first,last Range of elements.
   *
   * @warning Size of value type of the iterator must be equal to size of
   * unsigned char.
   */
  template <typename It>
  void write(It first, It last, std::forward_iterator_tag) {
    using T = typename std::iterator_traits<It>::value_type;
    // static_assert(sizeof(T) == sizeof(uchar),
    //               "Size of type not equals size of byte type.");

    auto s = static_cast<size_t>(std::distance(first, last));
    if (iwrite + s - 1 >= size())
      d.resize(size() + s);

    auto start = d.begin() + static_cast<std::ptrdiff_t>(iwrite);
    std::copy(first, last, start);
    iwrite += s;
  }

  /**
   * @brief Writes values from the buffer and writes to the T-type array.
   *
   * @code{.cpp}
   * dislua::buffer buf;
   * dislua::uchar arr[3] = { 0x5, 0xA0, 0xFF };
   * buf.write(arr, 3);
   * @endcode
   *
   * @tparam T Value type.
   * @param[in] arr Array.
   * @param[in] number Amount of elements.
   */
  template <typename T>
  void write(T *arr, size_t number) {
    size_t s = sizeof(T) * number;
    if (iwrite + s - 1 >= size())
      d.resize(size() + s);

    uchar *data = d.data();
    std::memcpy(data + iwrite, arr, s);
    iwrite += s;
  }

  /**
   * @brief Reads a value of the specified type from the buffer.
   *
   * @code{.cpp}
   * dislua::buffer buf{ 0xA, 0xBC };
   * std::cout << std::hex << std::uppercase
   *       << +buf.read(false) << ' '
   *       << buf.read<dislua::ushort>() << '\n';
   *
   * // output: A BC0A
   * @endcode
   *
   * @tparam T Variable type.
   * @param[in] next Move to the next index to be read.
   * @return T-type variable.
   *
   * @warning Type T doesn't have to be a pointer.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  template <typename T = uchar>
  T read(bool next = true) {
    static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");

    T val;
    read(&val, 1, next);
    return val;
  }

  /**
   * @brief Writes a variable of a specific type to the buffer.
   *
   * @code{.cpp}
   * dislua::buffer buf;
   * buf.write<dislua::uint>(1);
   * @endcode
   *
   * @tparam T Variable type
   * @param[in] val Variable.
   *
   * @warning Type T doesn't have to be a pointer.
   */
  template <typename T>
  void write(T val) {
    static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");
    write(&val, 1);
  }

  /**
   * @brief Writes values from the container to the buffer using two
   * iterators.
   *
   * @code{.cpp}
   * dislua::buffer buf;
   *
   * // input iterators
   * std::ifstream luac("./file.luac", std::ios::binary);
   * buf.write((std::istreambuf_iterator<char>(luac)),
   *           std::istreambuf_iterator<char>());
   *
   * // forward iterators
   * std::vector<unsigned char> a = {1, 2, 3};
   * buf.write(a.begin(), a.end());
   * @endcode
   *
   * @tparam It Iterator type.
   * @param[in] first,last Range of elements.
   */
  template <typename It>
  void write(It first, It last) {
    write(first, last, typename std::iterator_traits<It>::iterator_category());
  }
  /**
   * @brief Writes variables to the buffer from another buffer.
   *
   * @code{.cpp}
   * dislua::buffer a{1, 2, 3};
   * dislua::buffer b;
   *
   * b.write(a);
   * @endcode
   *
   * @param[in] buf Another buffer.
   */
  void write(buffer &buf) {
    write(buf.d.begin(), buf.d.end());
  }

  /// Resets the buffer.
  void reset() {
    reset_indices();
    d.clear();
  }
  /// Resets buffer indices.
  void reset_indices() {
    iread = iwrite = 0;
  }
  /// Returns a copy of the buffer container.
  decltype(d) copy_data() {
    return d;
  }
  /// Returns a size of the container.
  size_t size() {
    return d.size();
  }

  // Another types

  /**
   * @brief Reads ULEB128 (Unsigned Little Endian Base 128).
   *
   * @code{.cpp}
   *
   * dislua::buffer buf{ 0x90, 0x3 };
   * std::cout << buf.read_uleb128() << '\n';
   *
   * // output: 400
   * @endcode
   *
   * @param[in] next Move to the next index to be read.
   * @return ULEB128 value.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  uleb128 read_uleb128(bool next = true) {
    size_t idx = iread;
    uchar vb = read();
    auto val = static_cast<uleb128>(vb);

    if (val >= 0x80) {
      int sh = 0;
      val &= 0x7f;
      do {
        vb = read();
        val |= static_cast<uleb128>((vb & 0x7f) << (sh += 7));
      } while (vb >= 0x80);
    }

    if (!next)
      iread = idx;
    return val;
  }

  /**
   * @brief Reads ULEB128 values (Unsigned Little Endian Base 128) from the
   * buffer and writes to the array.
   *
   * @code{.cpp}
   * dislua::buffer buf{ 0x90, 0x3, 0xA5, 0x95, 0x3 };
   * dislua::uleb128 result[2];
   * buf.read_uleb128(result, 2);
   *
   * for (dislua::uleb128 val: result)
   *     std::cout << val << ' ';
   *
   * // output: 400 51877
   * @endcode
   *
   * @param[out] arr Array.
   * @param[in] number Amount of elements.
   * @param[in] next Move to the next index to be read.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  void read_uleb128(uleb128 *arr, size_t number, bool next = true) {
    size_t idx = iread;
    while (number--)
      *arr++ = read_uleb128();
    if (!next)
      iread = idx;
  }

  /**
   * @brief Reads top 32 bits of 33 bit ULEB128 (Unsigned Little Endian Base
   * 128) value from buffer.
   *
   * @param[in] next Move to the next index to be read.
   * @return ULEB128 value.
   *
   * @exception std::out_of_range An error occurred while exiting the container.
   */
  uleb128 read_uleb128_33(bool next = true) {
    size_t idx = iread;
    uchar vb = read();
    auto val = static_cast<uleb128>(vb >> 1);

    if (val >= 0x40) {
      int sh = -1;
      val &= 0x3f;
      do {
        vb = read();
        val |= static_cast<uleb128>((vb & 0x7f) << (sh += 7));
      } while (vb >= 0x80);
    }

    if (!next)
      iread = idx;
    return val;
  }

  /**
   * @brief Writes ULEB128 (Unsigned Little Endian Base 128).
   *
   * @code{.cpp}
   * dislua::buffer buf;
   * buf.write_uleb128(400);
   * @endcode
   *
   * @param[in] val ULEB128 value.
   */
  void write_uleb128(uleb128 val) {
    for (; val >= 0x80; val >>= 7)
      write(static_cast<uchar>((val & 0x7f) | 0x80));
    write(static_cast<uchar>(val));
  }

  /**
   * @brief Writes ULEB128 (Unsigned Little Endian Base 128) variables from the
   * buffer and writes to the array.
   *
   * @code{.cpp}
   * dislua::buffer buf;
   * dislua::buffer vals[2] = { 400, 51877 };
   * buf.write_uleb128(vals, 2);
   * @endcode
   *
   * @param[in] obj Array.
   * @param[in] number Amount of elements.
   */
  void write_uleb128(uleb128 *obj, size_t number) {
    while (number--)
      write_uleb128(*obj++);
  }

  /**
   * @brief Writes a 33 bit ULEB128 (Unsigned Little Endian Base 128).
   *
   * @code{.cpp}
   * dislua::buffer buf;
   * buf.write_uleb128_33(400);
   * @endcode
   *
   * @param[in] val
   * @param[in] isnum
   */
  void write_uleb128_33(uleb128 val, bool isnum = false) {
    size_t index = iwrite;
    write_uleb128(1 + 2 * val);
    uchar &v = d[index];
    if (isnum)
      v |= 1;
    else
      v &= 0xfe;
  }

  /// Read index.
  size_t iread = 0;
  /// Write index.
  size_t iwrite = 0;

  friend bool operator==(const buffer &lhs, const buffer &rhs) {
    return lhs.d == rhs.d;
  }

  template <typename T, typename A>
  friend bool operator==(const buffer &lhs, const std::vector<T, A> &rhs) {
    return lhs.d == rhs;
  }

  template <typename T, typename A>
  friend bool operator==(const std::vector<T, A> &lhs, const buffer &rhs) {
    return rhs == lhs;
  }
};
} // namespace dislua
#endif // DISLUA_BUFFER_H