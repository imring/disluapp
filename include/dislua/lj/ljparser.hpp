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

#ifndef DISLUA_LJ_PARSER_H
#define DISLUA_LJ_PARSER_H

#include "../const.hpp"
#include "../detail.hpp"
#include "../dump_info.hpp"
#include "ljconst.hpp"

/**
 * @brief LuaJIT namespace in the DisLua library.
 *
 * Bytecode dump format:
 * @code
 * dump   = header proto+ 0U
 * header = ESC 'L' 'J' versionB flagsU [namelenU nameB*]
 * proto  = lengthU pdata
 * pdata  = phead bcinsW* uvdataH* kgc* knum* [debugB*]
 * phead  = flagsB numparamsB framesizeB numuvB numkgcU numknU numbcU
 *          [debuglenU [firstlineU numlineU]]
 * kgc    = kgctypeU { ktab | (loU hiU) | (rloU rhiU iloU ihiU) | strB* }
 * knum   = intU0 | (loU1 hiU)
 * ktab   = narrayU nhashU karray* khash*
 * karray = ktabk
 * khash  = ktabk ktabk
 * ktabk  = ktabtypeU { intU | (loU hiU) | strB* }
 *
 * B = 8 bit, H = 16 bit, W = 32 bit, U = ULEB128 of W, U0/U1 = ULEB128 of W+1
 * @endcode
 */
namespace dislua::lj {
/// Class for parsing LuaJIT bytecode.
class parser : public dump_info {
  std::string read_to_zero() {
    std::string s;
    char c;
    while (c = buf.read<char>(), c != '\0')
      s.push_back(c);
    return s;
  }

  template <typename T>
  T read_with_uleb128() {
    constexpr size_t num = sizeof(T) / sizeof(uleb128);
    uleb128 vals[num];
    buf.read_uleb128(vals, num);
    return std::bit_cast<T>(vals);
  }

  template <typename T>
  static void write_with_uleb128(T val, buffer &buf) {
    constexpr size_t num = sizeof(T) / sizeof(uleb128);
    uleb128 vals[num];
    std::memcpy(vals, &val, sizeof(T));
    buf.write_uleb128(vals, num);
  }

  /// Read
  void read_header() {
    if (buf.read() != header::HEAD1
     || buf.read() != header::HEAD2
     || buf.read() != header::HEAD3)
      throw std::runtime_error("LuaJIT: Invalid header.");

    version = static_cast<uint>(buf.read());
    if (version != 2 && version != 1)
      throw std::runtime_error("LuaJIT: Unknown version.");

    uleb128 flags = buf.read_uleb128();
    header.flags = flags;

    flags &= ~dump_flags::be;
    flags &= ~dump_flags::strip;
    flags &= ~dump_flags::ffi;
    if (version == 2)
      flags &= ~dump_flags::fr2;
    if (flags)
      throw std::runtime_error("LuaJIT: Unknown header flags.");

    if ((header.flags & dump_flags::strip) == 0) {
      uleb128 len = buf.read_uleb128();
      header.debug_name.resize(len);
      buf.read(header.debug_name.begin(), header.debug_name.end());
    }
  }

  void read_bc_instructions(proto &pt, uleb128 size) {
    while (size--) {
      instruction ins = buf.read<instruction>();
      pt.ins.push_back(ins);
    }
  }

  void read_uv(proto &pt, uchar size) {
    while (size--) {
      ushort uv_data = buf.read<ushort>();
      pt.uv.push_back(uv_data);
    }
  }

  table_val_t read_ktabk() {
    uleb128 type = buf.read_uleb128();
    table_val_t value;

    switch (type) {
    case ktab::nil:
      value = nullptr;
      break;
    case ktab::fal:
      value = false;
      break;
    case ktab::tru:
      value = true;
      break;
    case ktab::integer: {
      uleb128 v = buf.read_uleb128();
      value = static_cast<leb128>(v);
      break;
    }
    case ktab::number: {
      uleb128 vals[2] = {0};
      buf.read_uleb128(vals, 2);
      value = std::bit_cast<double>(vals);
      break;
    }
    default: {
      std::string v;
      v.resize(type - ktab::string);
      buf.read(v.begin(), v.end());
      value = v;
      break;
    }
    }

    return value;
  }

  table_t read_ktab() {
    table_t table;
    uleb128 narray = buf.read_uleb128();
    uleb128 nhash = buf.read_uleb128();

    for (uleb128 i = 0; i < narray; ++i) {
      table_val_t value = read_ktabk();
      table.insert({static_cast<leb128>(i), value});
    }
    while (nhash--) {
      table_val_t key = read_ktabk();
      table_val_t value = read_ktabk();
      table.insert({key, value});
    }

    return table;
  }

  void read_kgc(proto &pt, uleb128 size) {
    uleb128 type;
    while (size--) {
      type = buf.read_uleb128();

      switch (type) {
      case kgc::child:
        pt.kgc.emplace_back(temp_protos.back());
        temp_protos.pop_back();
        break;
      case kgc::tab: {
        table_t table = read_ktab();
        pt.kgc.emplace_back(table);
        break;
      }
      case kgc::i64:
        pt.kgc.emplace_back(read_with_uleb128<long long>());
        break;
      case kgc::u64:
        pt.kgc.emplace_back(read_with_uleb128<unsigned long long>());
        break;
      case kgc::complex: {
        double x = read_with_uleb128<double>(), y = read_with_uleb128<double>();
        std::complex<double> z{x, y};
        pt.kgc.emplace_back(z);
        break;
      }
      default: {
        uleb128 length = type - kgc::string;
        std::string str(length, '\0');
        buf.read(str.begin(), str.end());
        pt.kgc.emplace_back(str);
        break;
      }
      }
    }
  }

  void read_knum(proto &pt, uleb128 size) {
    while (size--) {
      bool isnum = buf.read(false) & 1;

      uleb128 result[2] = {buf.read_uleb128_33(), 0};
      double v;
      if (isnum) {
        result[1] = buf.read_uleb128();
        v = std::bit_cast<double>(result);
      } else
        v = static_cast<double>(static_cast<leb128>(result[0]));
      pt.knum.push_back(v);
    }
  }

  void read_lineinfo(proto &pt, uleb128 size) {
    while (size--) {
      uint line;
      if (pt.numline >= 1 << 16)
        line = buf.read<uint>();
      else if (pt.numline >= 1 << 8)
        line = static_cast<uint>(buf.read<ushort>());
      else
        line = static_cast<uint>(buf.read());
      pt.lineinfo.push_back(pt.firstline + line);
    }
  }

  void read_uvname(proto &pt, uchar size) {
    while (size--)
      pt.uv_names.push_back(read_to_zero());
  }

  void read_varname(proto &pt) {
    size_t last = 0;
    uchar type;
    while (type = buf.read(), type != varnames::end) {
      varname info = {};
      info.type = type;

      if (type >= varnames::MAX) {
        buf.iread--;
        info.name = read_to_zero();
      }

      last = info.start = last + static_cast<size_t>(buf.read_uleb128());
      info.end = info.start + static_cast<size_t>(buf.read_uleb128());

      pt.varnames.push_back(info);
    }
  }

  void read_proto(proto &pt) {
    uleb128 sizekgc, sizekn, sizebc;
    uchar sizeuv;

    pt.flags = buf.read();
    pt.numparams = buf.read();
    pt.framesize = buf.read();
    sizeuv = buf.read();

    sizekgc = buf.read_uleb128();
    sizekn = buf.read_uleb128();
    sizebc = buf.read_uleb128();

    uchar flags = pt.flags;
    flags &= ~proto_flags::child;
    flags &= ~proto_flags::varargs;
    flags &= ~proto_flags::ffi;
    flags &= ~proto_flags::nojit;
    flags &= ~proto_flags::iloop;
    if (flags)
      throw std::runtime_error("LuaJIT: Unknown prototype flags.");

    uleb128 sizedbg = 0;
    if ((header.flags & dump_flags::strip) == 0) {
      sizedbg = buf.read_uleb128();
      if (sizedbg) {
        pt.firstline = buf.read_uleb128();
        pt.numline = buf.read_uleb128();
      }
    }

    read_bc_instructions(pt, sizebc);
    read_uv(pt, sizeuv);
    read_kgc(pt, sizekgc);
    read_knum(pt, sizekn);

    size_t sizedbg_read = buf.iread;
    if (sizedbg) {
      read_lineinfo(pt, sizebc);
      read_uvname(pt, sizeuv);
      read_varname(pt);
    }
    if (buf.iread - sizedbg_read != sizedbg)
      throw std::runtime_error("LuaJIT: Invalid debuginfo size.");
  }

  /// Write
  void write_header() {
    buf.write(header::HEAD1);
    buf.write(header::HEAD2);
    buf.write(header::HEAD3);

    buf.write(static_cast<uchar>(version));
    buf.write_uleb128(header.flags);

    if ((header.flags & dump_flags::strip) == 0) {
      buf.write_uleb128(static_cast<uleb128>(header.debug_name.size()));
      buf.write(header.debug_name.begin(), header.debug_name.end());
    }
  }

  void write_bc_instructions(proto &pt, buffer &ptbuf) {
    for (instruction &cins: pt.ins)
      ptbuf.write(cins);
  }

  void write_uv(proto &pt, buffer &ptbuf) {
    for (ushort &uv: pt.uv)
      ptbuf.write(uv);
  }

  void write_ktabk(const table_val_t &val, buffer &pthash) {
    std::visit(detail::overloaded{[&](std::nullptr_t) {
                                    pthash.write_uleb128(ktab::nil);
                                  },
                                  [&](bool arg) {
                                    pthash.write_uleb128(arg ? ktab::tru : ktab::fal);
                                  },
                                  [&](leb128 arg) {
                                    pthash.write_uleb128(ktab::integer);
                                    pthash.write_uleb128(static_cast<uleb128>(arg));
                                  },
                                  [&](double arg) {
                                    pthash.write_uleb128(ktab::number);
                                    write_with_uleb128(arg, pthash);
                                  },
                                  [&](std::string arg) {
                                    pthash.write_uleb128(ktab::string + static_cast<uleb128>(arg.size()));
                                    pthash.write(arg.begin(), arg.end());
                                  }},
               val);
  }

  void write_ktab(table_t &table, buffer &ptbuf) {
    buffer ptarray, pthash;
    table_t copy_table = table;

    leb128 i = 0;
    decltype(copy_table)::iterator it;
    while (it = copy_table.find(i++), it != copy_table.end()) {
      write_ktabk(it->second, ptarray);
      copy_table.erase(it->first);
    }

    for (const auto &[key, value]: copy_table) {
      write_ktabk(key, pthash);
      write_ktabk(value, pthash);
    }

    ptbuf.write_uleb128(static_cast<uleb128>(i - 1));
    ptbuf.write_uleb128(static_cast<uleb128>(copy_table.size()));
    ptbuf.write(ptarray);
    ptbuf.write(pthash);
  }

  void write_kgc(proto &pt, buffer &ptbuf) {
    for (kgc_t &kgc: pt.kgc)
      std::visit(detail::overloaded{[&](const proto_id &) {
                                      ptbuf.write_uleb128(kgc::child);
                                    },
                                    [&](table_t arg) {
                                      ptbuf.write_uleb128(kgc::tab);
                                      write_ktab(arg, ptbuf);
                                    },
                                    [&](long long arg) {
                                      ptbuf.write_uleb128(kgc::i64);
                                      write_with_uleb128(arg, ptbuf);
                                    },
                                    [&](unsigned long long arg) {
                                      ptbuf.write_uleb128(kgc::u64);
                                      write_with_uleb128(arg, ptbuf);
                                    },
                                    [&](std::complex<double> arg) {
                                      ptbuf.write_uleb128(kgc::complex);
                                      write_with_uleb128(arg, ptbuf);
                                    },
                                    [&](std::string arg) {
                                      ptbuf.write_uleb128(static_cast<uleb128>(arg.size()) + kgc::string);
                                      ptbuf.write(arg.begin(), arg.end());
                                    }},
                 kgc);
  }

  void write_knum(proto &pt, buffer &ptbuf) {
    for (double &val: pt.knum) {
      double sval = static_cast<double>(static_cast<int>(val)); // signed value
      if (detail::almost_equal(val, sval, 2))
        ptbuf.write_uleb128_33(static_cast<uleb128>(val));
      else {
        uleb128 v[2] = {0};
        std::memcpy(v, &val, sizeof(val));
        ptbuf.write_uleb128_33(v[0], true);
        ptbuf.write_uleb128(v[1]);
      }
    }
  }

  void write_lineinfo(proto &pt, buffer &ptdebug) {
    if (pt.lineinfo.size() != pt.ins.size())
      throw std::runtime_error("LuaJIT: Line number != instruction number");
    for (uint line: pt.lineinfo) {
      if (pt.numline >= 1 << 16)
        ptdebug.write(line - pt.firstline);
      else if (pt.numline >= 1 << 8)
        ptdebug.write(static_cast<ushort>(line - pt.firstline));
      else
        ptdebug.write(static_cast<uchar>(line - pt.firstline));
    }
  }

  void write_uvname(proto &pt, buffer &ptdebug) {
    if (pt.uv_names.size() != pt.uv.size())
      throw std::runtime_error("LuaJIT: Number of upvalue names != upvalue count");
    for (std::string &name: pt.uv_names) {
      ptdebug.write(name.begin(), name.end());
      ptdebug.write('\0');
    }
  }

  void write_varname(proto &pt, buffer &ptdebug) {
    size_t last = 0;
    for (varname &info: pt.varnames) {
      if (info.type >= varnames::MAX) {
        ptdebug.write(info.name.begin(), info.name.end());
        ptdebug.write('\0');
      } else
        ptdebug.write(info.type);
      ptdebug.write_uleb128(static_cast<uleb128>(info.start - last));
      last = info.start;
      ptdebug.write_uleb128(static_cast<uleb128>(info.end - info.start));
    }
    ptdebug.write(varnames::end);
  }

  void write_proto(proto &pt) {
    buffer ptbuf, ptdebug;
    size_t sizedebug = 0;

    ptbuf.write(pt.flags);
    ptbuf.write(pt.numparams);
    ptbuf.write(pt.framesize);
    ptbuf.write(static_cast<uchar>(pt.uv.size()));

    ptbuf.write_uleb128(static_cast<uleb128>(pt.kgc.size()));
    ptbuf.write_uleb128(static_cast<uleb128>(pt.knum.size()));
    ptbuf.write_uleb128(static_cast<uleb128>(pt.ins.size()));

    if ((header.flags & dump_flags::strip) == 0) {
      write_lineinfo(pt, ptdebug);
      write_uvname(pt, ptdebug);
      write_varname(pt, ptdebug);

      sizedebug = ptdebug.iwrite;
      ptbuf.write_uleb128(static_cast<uleb128>(sizedebug));
      if (sizedebug) {
        ptbuf.write_uleb128(pt.firstline);
        ptbuf.write_uleb128(pt.numline);
      }
    }

    write_bc_instructions(pt, ptbuf);
    write_uv(pt, ptbuf);
    write_kgc(pt, ptbuf);
    write_knum(pt, ptbuf);

    if (sizedebug)
      ptbuf.write(ptdebug);

    size_t len = ptbuf.iwrite;
    buf.write_uleb128(static_cast<uleb128>(len));
    buf.write(ptbuf);
  }

  std::vector<proto_id> temp_protos;

public:
  explicit parser(const buffer &_buf = {}) : dump_info(_buf) {}
  explicit parser(const dump_info &rv) : dump_info(rv) {}

  void read() override {
    buf.reset_indices();

    read_header();
    while (buf.iread < buf.size()) {
      if (!buf.read(false))
        break;

      uleb128 len = buf.read_uleb128();
      if (!len)
        throw std::runtime_error("LuaJIT: Prototype size == 0.");

      proto pt;
      size_t start = buf.iread;
      read_proto(pt);
      if (buf.iread - start != len)
        throw std::runtime_error("LuaJIT: Different prototype sizes.");
      temp_protos.emplace_back(temp_protos.size());
      protos.push_back(pt);
    }

    if (temp_protos.size() != 1)
      throw std::runtime_error("LuaJIT: Invalid prototype stack.");
    if (buf.iread + 1 > buf.size())
      throw std::runtime_error("LuaJIT: Something not read.");
    temp_protos.clear();

    buf.reset_indices();
  }

  void write() override {
    buf.reset();

    write_header();
    for (proto &pt: protos)
      write_proto(pt);
    buf.write('\0');

    buf.reset_indices();
  }

  compilers compiler() override {
    return compilers::luajit;
  }
};
} // namespace dislua::lj

#endif // DISLUA_LJ_PARSER_H