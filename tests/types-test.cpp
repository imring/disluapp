#include "gtest/gtest.h"

#include "dislua/types/instruction.hpp"
#include "dislua/types/kgc.hpp"
#include "dislua/types/proto.hpp"
#include "dislua/types/table.hpp"
#include "dislua/types/varname.hpp"

TEST(TYPES_TEST, INSTRUCTION) {
  dislua::instruction ins0, ins1(1, 2, 3), ins2(1, 2, 4, 3);

  ASSERT_FALSE(ins0 == ins1);
  ASSERT_FALSE(ins1 == ins2);
  ins2.b = 0;
  ASSERT_TRUE(ins1 == ins2);
}

TEST(TYPES_TEST, KGC) {
  using namespace std::complex_literals;
  dislua::kgc_t val0 = dislua::proto(), val1 = -1LL, val2 = 1ULL, val3 = 1i, val4 = "test";

  ASSERT_EQ(std::get<unsigned long long>(val2), std::abs(std::get<long long>(val1)));
  val0 = "test";
  ASSERT_EQ(val0, val4);

  std::complex<double> v = std::get<std::complex<double>>(val3);
  ASSERT_EQ(1, std::abs(v));
}

TEST(TYPES_TEST, TABLE) {
  dislua::table_t tab0 = {{1, "test"}, {2, nullptr}, {"index", false}}, tab1;
  tab1.insert({1, "test"});
  tab1[2] = nullptr;
  ASSERT_FALSE(tab0 == tab1);
  tab1["index"] = false;
  ASSERT_TRUE(tab0 == tab1);
}

TEST(TYPES_TEST, VARNAME) {
  dislua::varname vn0(1, 0, 5), vn1 = vn0;
  ASSERT_TRUE(vn0 == vn1);
  vn0.name = "test";
  vn0.type = static_cast<dislua::uchar>('t');
  ASSERT_FALSE(vn0 == vn1);
}

TEST(TYPES_TEST, PROTO) {}