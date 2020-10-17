#pragma once
#include <vector>
#include <string>
#include <limits>
#include <cstring>
#include <stdexcept>
#include <functional>
#include <type_traits>

#include "const.hpp"

namespace dislua {
  class buffer {
  public:
    buffer() = default;
    
    template<typename It>
    buffer(It first, It last) {
      write(first, last);
      reset_index();
    }
    
    buffer(std::initializer_list<byte> arr): buffer(arr.begin(), arr.end()) {}
    
    template<typename T = byte>
    T read(bool next = true) {
      static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");
      T val;
      read(&val, sizeof(T), next);
      return val;
    }
    
    void read(void *obj, size_t len, bool next = true) {
      index_better_size_throw(len);
      byte *data = container.data();
      memcpy(obj, data + index, len);
      if (next) ignore(len);
    }
    
    template<typename It>
    void read(It first, It last, bool next = true) {
      size_t len = last - first;
      index_better_size_throw(len);
      decltype(container.begin()) start = container.begin() + index;
      std::copy(start, start + len, first);
      if (next) ignore(len);
    }
    
    template<typename T>
    void read_to_zero(T &container, bool next = true) {
      size_t first = get_index();
      byte val = 0;
      while (true) {
        val = read();
        if (!val) break;
        container.push_back(val);
      }
      if(!next) index = first;
    }
    
    uleb128 read_uleb128(bool next = true) {
      size_t first = get_index();
      byte vb = read();
      uleb128 val = uleb128(vb);

      if (val >= 0x80) {
        int sh = 0;
        val &= 0x7f;
        do {
          vb = read();
          val |= ((vb & 0x7f) << (sh += 7));
        } while (vb >= 0x80);
      }

      if(!next) index = first;
      return val;
    }
    
    void read_uleb128(uleb128 *obj, size_t len, bool next = true) {
      size_t first = get_index();
      while(len--) *obj++ = read_uleb128();
      if(!next) index = first;
    }
    
    uleb128 read_uleb128_33(bool next = true) {
      size_t first = get_index();
      byte vb = read();
      uleb128 val = uleb128(vb >> 1);

      if (val >= 0x40) {
        int sh = -1;
        val &= 0x3f;
        do {
          vb = read();
          val |= ((vb & 0x7f) << (sh += 7));
        } while (vb >= 0x80);
      }

      if(!next) index = first;
      return val;
    }
    
    template<typename T>
    void write(T val) {
      static_assert(!std::is_pointer_v<T>, "This method doesn't support pointer");
      write(&val, sizeof(T));
    }
    
    void write(void *arr, size_t len) {
      if (index_better_size(len))
        container.resize(container.size() + len);
      byte *data = container.data();
      memcpy(data + index, arr, len);
      ignore(len);
    }
    
    template<typename It>
    void write(It first, It last) {
      using T = typename std::iterator_traits<It>::value_type;
      static_assert(sizeof(T) == sizeof(byte), "Size of type not equals size of byte type.");
      size_t len = last - first;
      if (index_better_size(len))
        container.resize(container.size() + len);
      std::copy(first, last, container.begin() + index);
      ignore(len);
    }
    
    void write(buffer &buf) {
      write(buf.container.begin(), buf.container.end());
    }
    
    void write_uleb128(uleb128 val) {
      for (; val >= 0x80; val >>= 7)
        write(byte((val & 0x7f) | 0x80));
      write(byte(val));
    }
    
    void write_uleb128(uleb128 *obj, size_t len) {
      while(len--)
        write_uleb128(*obj++);
    }
    
    void write_uleb128_33(uleb128 val, bool isnum = false) {
      size_t index = get_index();
      write_uleb128(1 + 2 * val);
      byte &v = container[index];
      if (isnum) v |= 1;
      else v &= 0xfe;

      if (val > std::numeric_limits<std::make_signed_t<uleb128>>::max())
        container[get_index() - 1] |= 0x10;
    }

    void clear() {
      container.clear();
      reset_index();
    }
    
    inline void reset_index() { index = 0; }
    inline void ignore(size_t offset) { index += offset; }
    inline const size_t get_index() const { return index; }
    inline bool index_better_size(size_t size = 0) const { return index + size - 1 >= container.size(); }
    inline std::vector<byte> get_copy_container() const { return container; }
  private:
    inline void index_better_size_throw(size_t size = 0) const {
      if (index_better_size(size))
        throw std::out_of_range("Index (" + std::to_string(index + size) + ") better than container size (" + std::to_string(container.size()) + ").");
    }

    std::vector<byte> container;
    size_t index = 0;
  };
};