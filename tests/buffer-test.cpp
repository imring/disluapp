#include <vector>

#include "gtest/gtest.h"

#include "dislua/buffer.hpp"

TEST(BUFFER_TEST, CONSTRUCT_ITERATOR) {
  std::vector<dislua::uchar> vec = {1, 2, 3};
  dislua::buffer buf(vec.begin(), vec.end());
  ASSERT_EQ(buf, vec);
}

TEST(BUFFER_TEST, CONSTRUCT_INITIALIZER_LIST) {
  std::vector<dislua::uchar> vec = {1, 2, 3};
  dislua::buffer buf0 = {1, 2, 3};
  dislua::buffer buf1 = {'\1', '\2', '\3'};

  ASSERT_EQ(buf0, vec);
  ASSERT_EQ(buf1, vec);
}

TEST(BUFFER_TEST, CONSTRUCT_BUFFER) {
  dislua::buffer buf0 = {1, 2, 3};
  dislua::buffer buf1 = buf0;

  ASSERT_EQ(buf0, buf1);
}

TEST(BUFFER_TEST, READ_ARRAY) {
  dislua::buffer buf = {0x10, 0x80, 0xD, 0x2};
  std::vector<dislua::uchar> result = {0, 0, 0};
  buf.read(result.data(), 1, false);
  buf.read(result.data() + 1, 2);

  static std::vector<dislua::uchar> eq = {0x10, 0x10, 0x80};
  ASSERT_EQ(result, eq);
}

TEST(BUFFER_TEST, READ_ITERATOR) {
  dislua::buffer buf = {0x10, 0x80, 0xD, 0x2};
  std::vector<dislua::uchar> result(3);
  buf.read(result.begin(), result.begin() + 1, false);
  buf.read(result.begin() + 1, result.end());

  static std::vector<dislua::uchar> eq = {0x10, 0x10, 0x80};
  ASSERT_EQ(result, eq);
}

TEST(BUFFER_TEST, WRITE_INPUT_ITERATOR) {
  std::istringstream in("\x01\x10\x80");
  dislua::buffer buf;
  buf.write((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());

  static std::vector<dislua::uchar> eq = {0x1, 0x10, 0x80};
  ASSERT_EQ(buf.copy_data(), eq);
}

TEST(BUFFER_TEST, WRITE_FORWARD_ITERATOR) {
  std::vector<dislua::uchar> v = {1, 2, 3};
  dislua::buffer buf;
  buf.write(v.begin(), v.end());

  ASSERT_EQ(buf, v);
}

TEST(BUFFER_TEST, WRITE_ARRAY) {
  dislua::buffer buf;
  std::vector<dislua::uchar> arr = {0x5, 0xA0, 0xFF};
  buf.write(arr.data(), 3);

  ASSERT_EQ(buf, arr);
}

TEST(BUFFER_TEST, READ_VAL) {
  dislua::buffer buf = {8, 1};
  ASSERT_EQ(8, buf.read(false));
  ASSERT_EQ(8, buf.read());

  buf.reset_indices();
  ASSERT_EQ(264, buf.read<dislua::ushort>(false));
  ASSERT_EQ(264, buf.read<dislua::ushort>());
}

TEST(BUFFER_TEST, WRITE_VAL) {
  dislua::buffer buf;
  buf.write<dislua::ushort>(264);

  ASSERT_EQ(8, buf.read());
  ASSERT_EQ(1, buf.read());
}

TEST(BUFFER_TEST, READ_ULEB128) {
  dislua::buffer buf = {0x90, 0x3};

  ASSERT_EQ(400, buf.read_uleb128(false));
  ASSERT_EQ(400, buf.read_uleb128());
}

TEST(BUFFER_TEST, READ_ULEB128_33) {
  dislua::buffer buf = {0x90, 0x3};

  ASSERT_EQ(200, buf.read_uleb128_33(false));
  ASSERT_EQ(200, buf.read_uleb128_33());
}

TEST(BUFFER_TEST, WRITE_ULEB128) {
  dislua::buffer buf;
  buf.write_uleb128(400);

  ASSERT_EQ(912, buf.read<dislua::ushort>(false));
  ASSERT_EQ(912, buf.read<dislua::ushort>());
}

TEST(BUFFER_TEST, WRITE_ULEB128_33) {
  dislua::buffer buf;
  buf.write_uleb128_33(200);

  ASSERT_EQ(912, buf.read<dislua::ushort>(false));
  ASSERT_EQ(912, buf.read<dislua::ushort>());
}