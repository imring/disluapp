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

#ifndef DISLUA_LJ_PARSER_H
#define DISLUA_LJ_PARSER_H

#include "../const.hpp"
#include "../dump_info.hpp"
#include "ljconst.hpp"

namespace dislua::lj {
class parser : public dump_info {
  std::string read_to_zero() {
    std::string s;
    char c;
    while (c = buf->read<char>(), c != '\0')
      s.push_back(c);
    return s;
  }

  /// Read
  void read_header() {
    if (buf->read() != header::HEAD1 || buf->read() != header::HEAD2 ||
        buf->read() != header::HEAD3)
      throw std::runtime_error("LuaJIT: Invalid header.");

    version = static_cast<uint>(buf->read());
    if (version != 2 && version != 1)
      throw std::runtime_error("LuaJIT: Unknown version.");

    uleb128 flags = buf->read_uleb128();
    header.flags = flags;

    flags &= DUMP_BE;
    flags &= DUMP_STRIP;
    flags &= DUMP_FFI;
    if (version == 2)
      flags &= DUMP_FR2;
    if (flags)
      throw std::runtime_error("LuaJIT: Unknown header flags.");

    if ((header.flags & DUMP_STRIP) == 0) {
      uleb128 len = buf->read_uleb128();
      header.debug_name.resize(len);
      buf->read(header.debug_name.begin(), header.debug_name.end());
    }
  }

  void read_bc_instructions(proto &pt, uleb128 size) {
    while (size--) {
      instruction ins = buf->read<instruction>();
      pt.ins.push_back(ins);
    }
  }

  void read_uv(proto &pt, uchar size) {
    while (size--) {
      ushort uv_data = buf->read<ushort>();
      pt.uv.push_back(uv_data);
    }
  }

  table_val_t read_ktabk() {
    uleb128 type = buf->read_uleb128();
    table_val_t value;

    if (type >= KTAB_STR) {
      std::string v;
      v.resize(type - KTAB_STR);
      buf->read(v.begin(), v.end());
      value = v;
    } else if (type == KTAB_NUM) {
      uleb128 vals[2] = {0};
      buf->read_uleb128(vals, 2);
      value = *pointer_cast<double *>(vals);
    } else if (type == KTAB_INT) {
      uleb128 v = buf->read_uleb128();
      value = leb128(v);
    } else if (type == KTAB_NIL)
      value = nullptr;
    else
      value = type == KTAB_TRUE;

    return value;
  }

  table_t read_ktab() {
    table_t table;
    uleb128 narray = buf->read_uleb128();
    uleb128 nhash = buf->read_uleb128();

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
    uleb128 type = KGC_CHILD;
    while (size--) {
      type = buf->read_uleb128();

      if (type == KGC_CHILD) {
        pt.kgc.push_back(temp_protos.back());
        temp_protos.pop_back();
      } else if (type == KGC_TAB) {
        table_t table = read_ktab();
        pt.kgc.push_back(table);
      } else if (type == KGC_I64) {
        long long v = 0;
        buf->read_uleb128(pointer_cast<uleb128 *>(&v),
                          sizeof(long long) / sizeof(uleb128));
        pt.kgc.push_back(v);
      } else if (type == KGC_U64) {
        unsigned long long v = 0;
        buf->read_uleb128(pointer_cast<uleb128 *>(&v),
                          sizeof(unsigned long long) / sizeof(uleb128));
        pt.kgc.push_back(v);
      } else if (type == KGC_COMPLEX) {
        std::complex<double> v;
        buf->read_uleb128(pointer_cast<uleb128 *>(&v),
                          sizeof(std::complex<double>) / sizeof(uleb128));
        pt.kgc.push_back(v);
      } else if (type >= KGC_STR) {
        uint length = type - KGC_STR;
        std::string str;
        str.resize(length);
        buf->read(str.begin(), str.end());
        pt.kgc.push_back(str);
      }
    }
  }

  void read_knum(proto &pt, uleb128 size) {
    while (size--) {
      bool isnum = buf->read(false) & 1;

      uleb128 result[2] = {buf->read_uleb128_33(), 0};
      double v = 0;
      if (isnum) {
        result[1] = buf->read_uleb128();
        v = *pointer_cast<double *>(result);
      } else
        v = static_cast<double>(static_cast<leb128>(result[0]));
      pt.knum.push_back(v);
    }
  }

  void read_lineinfo(proto &pt, uleb128 size) {
    while (size--) {
      uint line = 0;
      if (pt.numline >= 1 << 16)
        line = buf->read<uint>();
      else if (pt.numline >= 1 << 8)
        line = static_cast<uint>(buf->read<ushort>());
      else
        line = static_cast<uint>(buf->read());
      pt.lineinfo.push_back(pt.firstline + line);
    }
  }

  void read_uvname(proto &pt, uchar size) {
    while (size--)
      pt.uv_names.push_back(read_to_zero());
  }

  void read_varname(proto &pt) {
    size_t last = 0;
    uchar type = 0;
    while (type = buf->read(), type != VARNAME_END) {
      varname info = {};
      info.type = type;

      if (type >= VARNAME__MAX) {
        buf->iread--;
        info.name = read_to_zero();
      }

      last = info.start = last + static_cast<size_t>(buf->read_uleb128());
      info.end = info.start + static_cast<size_t>(buf->read_uleb128());

      pt.varnames.push_back(info);
    }
  }

  void read_proto(proto &pt) {
    uleb128 sizekgc, sizekn, sizebc;
    uchar sizeuv;

    pt.flags = buf->read();
    pt.numparams = buf->read();
    pt.framesize = buf->read();
    sizeuv = buf->read();

    sizekgc = buf->read_uleb128();
    sizekn = buf->read_uleb128();
    sizebc = buf->read_uleb128();

    uchar flags = pt.flags;
    flags &= PROTO_CHILD;
    flags &= PROTO_VARARG;
    flags &= PROTO_FFI;
    flags &= PROTO_NOJIT;
    flags &= PROTO_ILOOP;
    if (flags)
      throw std::runtime_error("LuaJIT: Unknown prototype flags.");

    uleb128 sizedbg = 0;
    if ((header.flags & DUMP_STRIP) == 0) {
      sizedbg = buf->read_uleb128();
      if (sizedbg) {
        pt.firstline = buf->read_uleb128();
        pt.numline = buf->read_uleb128();
      }
    }

    read_bc_instructions(pt, sizebc);
    read_uv(pt, sizeuv);
    read_kgc(pt, sizekgc);
    read_knum(pt, sizekn);

    size_t sizedbg_read = buf->iread;
    if (sizedbg) {
      read_lineinfo(pt, sizebc);
      read_uvname(pt, sizeuv);
      read_varname(pt);
    }
    if (buf->iread - sizedbg_read != sizedbg)
      throw std::runtime_error("LuaJIT: Invalid debuginfo size.");
  }

  /// Write
  void write_header() {
    buf->write(header::HEAD1);
    buf->write(header::HEAD2);
    buf->write(header::HEAD3);

    buf->write(static_cast<uchar>(version));
    buf->write_uleb128(header.flags);

    if ((header.flags & DUMP_STRIP) == 0) {
      buf->write_uleb128(static_cast<uleb128>(header.debug_name.size()));
      buf->write(header.debug_name.begin(), header.debug_name.end());
    }
  }

  void write_bc_instructions(proto &pt, buffer &ptbuf) {
    for (instruction &cins : pt.ins)
      ptbuf.write(cins);
  }

  void write_uv(proto &pt, buffer &ptbuf) {
    for (ushort &uv : pt.uv)
      ptbuf.write(uv);
  }

  void write_ktabk(const table_val_t &val, buffer &pthash) {
    std::visit(
        overloaded{[&](std::nullptr_t) { pthash.write_uleb128(KTAB_NIL); },
                   [&](bool arg) {
                     pthash.write_uleb128(arg ? KTAB_TRUE : KTAB_FALSE);
                   },
                   [&](leb128 arg) {
                     pthash.write_uleb128(KTAB_INT);
                     pthash.write_uleb128(static_cast<uleb128>(arg));
                   },
                   [&](double arg) {
                     pthash.write_uleb128(KTAB_NUM);
                     pthash.write_uleb128(pointer_cast<uleb128 *>(&arg), 2);
                   },
                   [&](std::string arg) {
                     pthash.write_uleb128(KTAB_STR +
                                          static_cast<uleb128>(arg.size()));
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

    for (table_t::value_type &kv : copy_table) {
      write_ktabk(kv.first, pthash);
      write_ktabk(kv.second, pthash);
    }

    ptbuf.write_uleb128(static_cast<uleb128>(i - 1));
    ptbuf.write_uleb128(static_cast<uleb128>(copy_table.size()));
    ptbuf.write(ptarray);
    ptbuf.write(pthash);
  }

  void write_kgc(proto &pt, buffer &ptbuf) {
    for (kgc_t &kgc : pt.kgc)
      std::visit(
          overloaded{[&](proto) { ptbuf.write_uleb128(KGC_CHILD); },
                     [&](table_t arg) {
                       ptbuf.write_uleb128(KGC_TAB);
                       write_ktab(arg, ptbuf);
                     },
                     [&](long long arg) {
                       ptbuf.write_uleb128(KGC_I64);
                       ptbuf.write_uleb128(pointer_cast<uleb128 *>(&arg),
                                           sizeof(long long) / sizeof(uleb128));
                     },
                     [&](unsigned long long arg) {
                       ptbuf.write_uleb128(KGC_U64);
                       ptbuf.write_uleb128(pointer_cast<uleb128 *>(&arg),
                                           sizeof(unsigned long long) /
                                               sizeof(uleb128));
                     },
                     [&](std::complex<double> arg) {
                       ptbuf.write_uleb128(KGC_COMPLEX);
                       ptbuf.write_uleb128(pointer_cast<uleb128 *>(&arg),
                                           sizeof(std::complex<double>) /
                                               sizeof(uleb128));
                     },
                     [&](std::string arg) {
                       ptbuf.write_uleb128(static_cast<uleb128>(arg.size()) +
                                           KGC_STR);
                       ptbuf.write(arg.begin(), arg.end());
                     }},
          kgc);
  }

  void write_knum(proto &pt, buffer &ptbuf) {
    for (double &val : pt.knum) {
      if (almost_equal(std::fmod(val, 1), 0.0, 2))
        ptbuf.write_uleb128_33(static_cast<uleb128>(val));
      else {
        uleb128 *v = pointer_cast<uleb128 *>(&val);
        ptbuf.write_uleb128_33(v[0], true);
        ptbuf.write_uleb128(v[1]);
      }
    }
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

    if ((header.flags & DUMP_STRIP) == 0) {
      // write_lineinfo(pt, ptdebug);
      // write_uvname(pt, ptdebug);
      // write_varname(pt, ptdebug);

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
    buf->write_uleb128(static_cast<uleb128>(len));
    buf->write(ptbuf);
  }

  std::vector<proto> temp_protos;

public:
  parser(buffer &_buf) : dump_info(_buf) {}
  parser(const dump_info &rv) : dump_info(rv) {}
  ~parser() {}

  void read() {
    buf->reset_indices();

    read_header();
    while (buf->iread < buf->size()) {
      if (!buf->read(false))
        break;

      uleb128 len = buf->read_uleb128();
      if (!len)
        throw std::runtime_error("LuaJIT: Prototype size == 0.");

      proto pt;
      size_t start = buf->iread;
      read_proto(pt);
      if (buf->iread - start != len)
        throw std::runtime_error("LuaJIT: Different prototype sizes.");
      temp_protos.push_back(pt);
      protos.push_back(pt);
    }

    if (temp_protos.size() != 1)
      throw std::runtime_error("LuaJit: Invalid prototype stack.");
    if (buf->iread + 1 > buf->size())
      throw std::runtime_error("LuaJIT: Something not read.");
    temp_protos.clear();

    buf->reset_indices();
  }

  void write() {
    buf->reset();

    write_header();
    for (proto &pt : protos)
      write_proto(pt);
    buf->write('\0');

    buf->reset_indices();
  }

  compilers compiler() { return DISLUA_LUAJIT; }
};
} // namespace dislua::lj

#endif // DISLUA_LJ_PARSER_H