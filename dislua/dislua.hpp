#pragma once
#include "const.hpp"
#include "buffer.hpp"
#include "interface.hpp"

#include "luajit/parser.hpp"

namespace dislua {
  template<typename T>
  static interface *read_current(buffer &buf) {
    static_assert(std::is_base_of_v<interface, T> && !std::is_same_v<interface, T>, "This type is not a dislua parser.");
    interface *result = new T(buf);
    result->read();
    return result;
  }
  
  static interface *read(buffer &buf) {
    interface *in = nullptr;
    std::string errors;

#define CHECK_COMPILER(T, ENUM) \
  try { \
    in = read_current<T>(buf); \
    if (in->compiler == ENUM) return in; \
  } catch (...) { delete in; in = nullptr; }

    CHECK_COMPILER(lj::parser, COMPILER_LUAJIT);
    
#undef CHECK_COMPILER

    return in;
  }
};