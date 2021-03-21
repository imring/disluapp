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

#ifndef DISLUA_LJ_CONST_H
#define DISLUA_LJ_CONST_H

#include <string>
#include <utility>

#include "../const.hpp"

namespace dislua::lj {
namespace header {
constexpr uchar HEAD1 = 0x1b;
constexpr uchar HEAD2 = 0x4c;
constexpr uchar HEAD3 = 0x4a;
} // namespace header

enum dump_flags : uint {
  DUMP_BE = 0b1,
  DUMP_STRIP = 0b10,
  DUMP_FFI = 0b100,
  DUMP_FR2 = 0b1000 // only for luajit v2.
};

enum proto_flags : uchar {
  PROTO_CHILD = 0b1,
  PROTO_VARARG = 0b10,
  PROTO_FFI = 0b100,
  PROTO_NOJIT = 0b1000,
  PROTO_ILOOP = 0b10000
};

enum kgc : uleb128 {
  KGC_CHILD,
  KGC_TAB,
  KGC_I64,
  KGC_U64,
  KGC_COMPLEX,
  KGC_STR
};

enum ktab : uleb128 {
  KTAB_NIL,
  KTAB_FALSE,
  KTAB_TRUE,
  KTAB_INT,
  KTAB_NUM,
  KTAB_STR
};

enum varnames : uchar {
  VARNAME_END,
  VARNAME_FOR_IDX,   // (for index)
  VARNAME_FOR_STOP,  // (for limit)
  VARNAME_FOR_STEP,  // (for step)
  VARNAME_FOR_GEN,   // (for generator)
  VARNAME_FOR_STATE, // (for state)
  VARNAME_FOR_CTL,   // (for control)
  VARNAME__MAX
};

enum bcmode {
  BCM___, // BCMnone
  BCMdst,
  BCMbase,
  BCMvar,
  BCMrbase,
  BCMuv, /* Mode A must be <= 7 */
  BCMlit,
  BCMlits,
  BCMpri,
  BCMnum,
  BCMstr,
  BCMtab,
  BCMfunc,
  BCMjump,
  BCMcdata,
  BCM_max
};

namespace v1 {
// https://github.com/LuaJIT/LuaJIT/blob/v2.0/src/lj_bc.h#L71
#define BCDEF(_)                                                               \
  /* Comparison ops. ORDER OPR. */                                             \
  _(ISLT, var, ___, var, lt)                                                   \
  _(ISGE, var, ___, var, lt)                                                   \
  _(ISLE, var, ___, var, le)                                                   \
  _(ISGT, var, ___, var, le)                                                   \
                                                                               \
  _(ISEQV, var, ___, var, eq)                                                  \
  _(ISNEV, var, ___, var, eq)                                                  \
  _(ISEQS, var, ___, str, eq)                                                  \
  _(ISNES, var, ___, str, eq)                                                  \
  _(ISEQN, var, ___, num, eq)                                                  \
  _(ISNEN, var, ___, num, eq)                                                  \
  _(ISEQP, var, ___, pri, eq)                                                  \
  _(ISNEP, var, ___, pri, eq)                                                  \
                                                                               \
  /* Unary test and copy ops. */                                               \
  _(ISTC, dst, ___, var, ___)                                                  \
  _(ISFC, dst, ___, var, ___)                                                  \
  _(IST, ___, ___, var, ___)                                                   \
  _(ISF, ___, ___, var, ___)                                                   \
                                                                               \
  /* Unary ops. */                                                             \
  _(MOV, dst, ___, var, ___)                                                   \
  _(NOT, dst, ___, var, ___)                                                   \
  _(UNM, dst, ___, var, unm)                                                   \
  _(LEN, dst, ___, var, len)                                                   \
                                                                               \
  /* Binary ops. ORDER OPR. VV last, POW must be next. */                      \
  _(ADDVN, dst, var, num, add)                                                 \
  _(SUBVN, dst, var, num, sub)                                                 \
  _(MULVN, dst, var, num, mul)                                                 \
  _(DIVVN, dst, var, num, div)                                                 \
  _(MODVN, dst, var, num, mod)                                                 \
                                                                               \
  _(ADDNV, dst, var, num, add)                                                 \
  _(SUBNV, dst, var, num, sub)                                                 \
  _(MULNV, dst, var, num, mul)                                                 \
  _(DIVNV, dst, var, num, div)                                                 \
  _(MODNV, dst, var, num, mod)                                                 \
                                                                               \
  _(ADDVV, dst, var, var, add)                                                 \
  _(SUBVV, dst, var, var, sub)                                                 \
  _(MULVV, dst, var, var, mul)                                                 \
  _(DIVVV, dst, var, var, div)                                                 \
  _(MODVV, dst, var, var, mod)                                                 \
                                                                               \
  _(POW, dst, var, var, pow)                                                   \
  _(CAT, dst, rbase, rbase, concat)                                            \
                                                                               \
  /* Constant ops. */                                                          \
  _(KSTR, dst, ___, str, ___)                                                  \
  _(KCDATA, dst, ___, cdata, ___)                                              \
  _(KSHORT, dst, ___, lits, ___)                                               \
  _(KNUM, dst, ___, num, ___)                                                  \
  _(KPRI, dst, ___, pri, ___)                                                  \
  _(KNIL, base, ___, base, ___)                                                \
                                                                               \
  /* Upvalue and function ops. */                                              \
  _(UGET, dst, ___, uv, ___)                                                   \
  _(USETV, uv, ___, var, ___)                                                  \
  _(USETS, uv, ___, str, ___)                                                  \
  _(USETN, uv, ___, num, ___)                                                  \
  _(USETP, uv, ___, pri, ___)                                                  \
  _(UCLO, rbase, ___, jump, ___)                                               \
  _(FNEW, dst, ___, func, gc)                                                  \
                                                                               \
  /* Table ops. */                                                             \
  _(TNEW, dst, ___, lit, gc)                                                   \
  _(TDUP, dst, ___, tab, gc)                                                   \
  _(GGET, dst, ___, str, index)                                                \
  _(GSET, var, ___, str, newindex)                                             \
  _(TGETV, dst, var, var, index)                                               \
  _(TGETS, dst, var, str, index)                                               \
  _(TGETB, dst, var, lit, index)                                               \
  _(TSETV, var, var, var, newindex)                                            \
  _(TSETS, var, var, str, newindex)                                            \
  _(TSETB, var, var, lit, newindex)                                            \
  _(TSETM, base, ___, num, newindex)                                           \
                                                                               \
  /* Calls and vararg handling. T = tail call. */                              \
  _(CALLM, base, lit, lit, call)                                               \
  _(CALL, base, lit, lit, call)                                                \
  _(CALLMT, base, ___, lit, call)                                              \
  _(CALLT, base, ___, lit, call)                                               \
  _(ITERC, base, lit, lit, call)                                               \
  _(ITERN, base, lit, lit, call)                                               \
  _(VARG, base, lit, lit, ___)                                                 \
  _(ISNEXT, base, ___, jump, ___)                                              \
                                                                               \
  /* Returns. */                                                               \
  _(RETM, base, ___, lit, ___)                                                 \
  _(RET, rbase, ___, lit, ___)                                                 \
  _(RET0, rbase, ___, lit, ___)                                                \
  _(RET1, rbase, ___, lit, ___)                                                \
                                                                               \
  /* Loops and branches. I/J = interp/JIT, I/C/L = init/call/loop. */          \
  _(FORI, base, ___, jump, ___)                                                \
  _(JFORI, base, ___, jump, ___)                                               \
                                                                               \
  _(FORL, base, ___, jump, ___)                                                \
  _(IFORL, base, ___, jump, ___)                                               \
  _(JFORL, base, ___, lit, ___)                                                \
                                                                               \
  _(ITERL, base, ___, jump, ___)                                               \
  _(IITERL, base, ___, jump, ___)                                              \
  _(JITERL, base, ___, lit, ___)                                               \
                                                                               \
  _(LOOP, rbase, ___, jump, ___)                                               \
  _(ILOOP, rbase, ___, jump, ___)                                              \
  _(JLOOP, rbase, ___, lit, ___)                                               \
                                                                               \
  _(JMP, rbase, ___, jump, ___)                                                \
                                                                               \
  /* Function headers. I/J = interp/JIT, F/V/C = fixarg/vararg/C func. */      \
  _(FUNCF, rbase, ___, ___, ___)                                               \
  _(IFUNCF, rbase, ___, ___, ___)                                              \
  _(JFUNCF, rbase, ___, lit, ___)                                              \
  _(FUNCV, rbase, ___, ___, ___)                                               \
  _(IFUNCV, rbase, ___, ___, ___)                                              \
  _(JFUNCV, rbase, ___, lit, ___)                                              \
  _(FUNCC, rbase, ___, ___, ___)                                               \
  _(FUNCCW, rbase, ___, ___, ___)

enum bcops : uchar {
#define BCENUM(name, ma, mb, mc, mm) BC_##name,
  BCDEF(BCENUM)
#undef BCENUM
      BC__MAX
};

static const std::pair<std::string, int> opcodes[] = {
#define BCOPCODES(name, ma, mb, mc, mm)                                        \
  {#name, BCM##ma | (BCM##mb << 3) | (BCM##mc << 7)},
    BCDEF(BCOPCODES)
#undef BCOPCODES
};

#undef BCDEF
} // namespace v1

namespace v2 {
// https://github.com/LuaJIT/LuaJIT/blob/v2.1/src/lj_bc.h#L71
#define BCDEF(_)                                                               \
  /* Comparison ops. ORDER OPR. */                                             \
  _(ISLT, var, ___, var, lt)                                                   \
  _(ISGE, var, ___, var, lt)                                                   \
  _(ISLE, var, ___, var, le)                                                   \
  _(ISGT, var, ___, var, le)                                                   \
                                                                               \
  _(ISEQV, var, ___, var, eq)                                                  \
  _(ISNEV, var, ___, var, eq)                                                  \
  _(ISEQS, var, ___, str, eq)                                                  \
  _(ISNES, var, ___, str, eq)                                                  \
  _(ISEQN, var, ___, num, eq)                                                  \
  _(ISNEN, var, ___, num, eq)                                                  \
  _(ISEQP, var, ___, pri, eq)                                                  \
  _(ISNEP, var, ___, pri, eq)                                                  \
                                                                               \
  /* Unary test and copy ops. */                                               \
  _(ISTC, dst, ___, var, ___)                                                  \
  _(ISFC, dst, ___, var, ___)                                                  \
  _(IST, ___, ___, var, ___)                                                   \
  _(ISF, ___, ___, var, ___)                                                   \
  _(ISTYPE, var, ___, lit, ___)                                                \
  _(ISNUM, var, ___, lit, ___)                                                 \
                                                                               \
  /* Unary ops. */                                                             \
  _(MOV, dst, ___, var, ___)                                                   \
  _(NOT, dst, ___, var, ___)                                                   \
  _(UNM, dst, ___, var, unm)                                                   \
  _(LEN, dst, ___, var, len)                                                   \
                                                                               \
  /* Binary ops. ORDER OPR. VV last, POW must be next. */                      \
  _(ADDVN, dst, var, num, add)                                                 \
  _(SUBVN, dst, var, num, sub)                                                 \
  _(MULVN, dst, var, num, mul)                                                 \
  _(DIVVN, dst, var, num, div)                                                 \
  _(MODVN, dst, var, num, mod)                                                 \
                                                                               \
  _(ADDNV, dst, var, num, add)                                                 \
  _(SUBNV, dst, var, num, sub)                                                 \
  _(MULNV, dst, var, num, mul)                                                 \
  _(DIVNV, dst, var, num, div)                                                 \
  _(MODNV, dst, var, num, mod)                                                 \
                                                                               \
  _(ADDVV, dst, var, var, add)                                                 \
  _(SUBVV, dst, var, var, sub)                                                 \
  _(MULVV, dst, var, var, mul)                                                 \
  _(DIVVV, dst, var, var, div)                                                 \
  _(MODVV, dst, var, var, mod)                                                 \
                                                                               \
  _(POW, dst, var, var, pow)                                                   \
  _(CAT, dst, rbase, rbase, concat)                                            \
                                                                               \
  /* Constant ops. */                                                          \
  _(KSTR, dst, ___, str, ___)                                                  \
  _(KCDATA, dst, ___, cdata, ___)                                              \
  _(KSHORT, dst, ___, lits, ___)                                               \
  _(KNUM, dst, ___, num, ___)                                                  \
  _(KPRI, dst, ___, pri, ___)                                                  \
  _(KNIL, base, ___, base, ___)                                                \
                                                                               \
  /* Upvalue and function ops. */                                              \
  _(UGET, dst, ___, uv, ___)                                                   \
  _(USETV, uv, ___, var, ___)                                                  \
  _(USETS, uv, ___, str, ___)                                                  \
  _(USETN, uv, ___, num, ___)                                                  \
  _(USETP, uv, ___, pri, ___)                                                  \
  _(UCLO, rbase, ___, jump, ___)                                               \
  _(FNEW, dst, ___, func, gc)                                                  \
                                                                               \
  /* Table ops. */                                                             \
  _(TNEW, dst, ___, lit, gc)                                                   \
  _(TDUP, dst, ___, tab, gc)                                                   \
  _(GGET, dst, ___, str, index)                                                \
  _(GSET, var, ___, str, newindex)                                             \
  _(TGETV, dst, var, var, index)                                               \
  _(TGETS, dst, var, str, index)                                               \
  _(TGETB, dst, var, lit, index)                                               \
  _(TGETR, dst, var, var, index)                                               \
  _(TSETV, var, var, var, newindex)                                            \
  _(TSETS, var, var, str, newindex)                                            \
  _(TSETB, var, var, lit, newindex)                                            \
  _(TSETM, base, ___, num, newindex)                                           \
  _(TSETR, var, var, var, newindex)                                            \
                                                                               \
  /* Calls and vararg handling. T = tail call. */                              \
  _(CALLM, base, lit, lit, call)                                               \
  _(CALL, base, lit, lit, call)                                                \
  _(CALLMT, base, ___, lit, call)                                              \
  _(CALLT, base, ___, lit, call)                                               \
  _(ITERC, base, lit, lit, call)                                               \
  _(ITERN, base, lit, lit, call)                                               \
  _(VARG, base, lit, lit, ___)                                                 \
  _(ISNEXT, base, ___, jump, ___)                                              \
                                                                               \
  /* Returns. */                                                               \
  _(RETM, base, ___, lit, ___)                                                 \
  _(RET, rbase, ___, lit, ___)                                                 \
  _(RET0, rbase, ___, lit, ___)                                                \
  _(RET1, rbase, ___, lit, ___)                                                \
                                                                               \
  /* Loops and branches. I/J = interp/JIT, I/C/L = init/call/loop. */          \
  _(FORI, base, ___, jump, ___)                                                \
  _(JFORI, base, ___, jump, ___)                                               \
                                                                               \
  _(FORL, base, ___, jump, ___)                                                \
  _(IFORL, base, ___, jump, ___)                                               \
  _(JFORL, base, ___, lit, ___)                                                \
                                                                               \
  _(ITERL, base, ___, jump, ___)                                               \
  _(IITERL, base, ___, jump, ___)                                              \
  _(JITERL, base, ___, lit, ___)                                               \
                                                                               \
  _(LOOP, rbase, ___, jump, ___)                                               \
  _(ILOOP, rbase, ___, jump, ___)                                              \
  _(JLOOP, rbase, ___, lit, ___)                                               \
                                                                               \
  _(JMP, rbase, ___, jump, ___)                                                \
                                                                               \
  /* Function headers. I/J = interp/JIT, F/V/C = fixarg/vararg/C func. */      \
  _(FUNCF, rbase, ___, ___, ___)                                               \
  _(IFUNCF, rbase, ___, ___, ___)                                              \
  _(JFUNCF, rbase, ___, lit, ___)                                              \
  _(FUNCV, rbase, ___, ___, ___)                                               \
  _(IFUNCV, rbase, ___, ___, ___)                                              \
  _(JFUNCV, rbase, ___, lit, ___)                                              \
  _(FUNCC, rbase, ___, ___, ___)                                               \
  _(FUNCCW, rbase, ___, ___, ___)

enum bcops : uchar {
#define BCENUM(name, ma, mb, mc, mm) BC_##name,
  BCDEF(BCENUM)
#undef BCENUM
      BC__MAX
};

static const std::pair<std::string, int> opcodes[] = {
#define BCOPCODES(name, ma, mb, mc, mm)                                        \
  {#name, BCM##ma | (BCM##mb << 3) | (BCM##mc << 7)},
    BCDEF(BCOPCODES)
#undef BCOPCODES
};

#undef BCDEF
} // namespace v2
} // namespace dislua::lj

#endif // DISLUA_LJ_CONST_H