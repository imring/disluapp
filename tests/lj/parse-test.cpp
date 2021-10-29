#include "gtest/gtest.h"

#include "dislua/dislua.hpp"

namespace lj = dislua::lj;

TEST(LUAJIT_TEST, HEADER_PARSE) {
  const dislua::uint version = 2;
  const dislua::uint flags = lj::dump_flags::be;
  const std::string debug_name = "test";

  lj::parser info;
  info.version = version;
  info.header = {
    .flags = flags,
    .debug_name = debug_name
  };
  info.protos.emplace_back(); // empty prototype

  info.write();
  ASSERT_NO_THROW(info.read());

  EXPECT_EQ(info.version, version);
  EXPECT_EQ(info.header.flags, flags);
  EXPECT_EQ(info.header.debug_name, debug_name);
}

TEST(LUAJIT_TEST, PROTO_PARSE) {
  const std::vector<dislua::instruction> ins{
    { lj::v2::bcops::KSTR, 0, 0 },
    { lj::v2::bcops::RET0, 0, 0 }
  };
  const dislua::proto p = {
    .flags = lj::proto_flags::varargs,
    .numparams = 3,
    .framesize = 1,

    // debug
    .firstline = 0,
    .numline = 260,

    .ins = ins,
    .kgc = { "test" },
    .knum = { 1.0, 3.125 },

    // debug
    .lineinfo = { 1, 2 }
  };

  lj::parser info;
  info.version = 2;
  info.header = {
    .flags = lj::dump_flags::be,
    .debug_name =  "test"
  };
  info.protos.push_back(p);

  info.write();
  ASSERT_NO_THROW(info.read());
  ASSERT_EQ(info.protos.front(), p);
}

TEST(LUAJIT_TEST, HEADER_UNK_VERSION) {
  lj::parser info;
  info.version = 3;
  info.header.flags = lj::dump_flags::strip;
  info.protos.emplace_back(); // empty prototype

  info.write();
  ASSERT_THROW(info.read(), std::runtime_error);
}

TEST(LUAJIT_TEST, HEADER_UNK_FLAGS) {
  lj::parser info;
  info.version = 2;

  // in header
  info.header.flags = 0b10000;
  info.protos.emplace_back(); // empty prototype
  info.write();
  ASSERT_THROW(info.read(), std::runtime_error);

  // in prototype
  info.header.flags = lj::dump_flags::strip;
  info.protos.pop_back();
  info.protos.push_back({ .flags = 0b100000 });
  info.write();
  ASSERT_THROW(info.read(), std::runtime_error);
}