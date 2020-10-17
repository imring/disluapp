#pragma once

namespace dislua {
  using byte  = unsigned char;
  using word  = unsigned short;
  using dword = unsigned int;

  using uleb128 = dword;

  enum compilers {
    COMPILER_UNKNOWN = -1,
    COMPILER_LUAJIT,
  };

  enum compiler_versions {
    CV_UNKNOWN = -1,
    CV_LUAJIT1,
    CV_LUAJIT2
  };
};