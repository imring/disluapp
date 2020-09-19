#pragma once

namespace dislua {
    using byte  = unsigned char;
    using word  = unsigned short;
    using dword = unsigned long;

    using uleb128 = dword;

    /*!
     * В данном перечислении представлен список версий компиляторов, поддерживаемых данной библиотекой.
     * В случае, если используется неизвестный компилятор, используется COMPILER_UNKNOWN.		
     */
    enum COMPILERS {
        COMPILER_UNKNOWN = -1,
        COMPILER_LUAJIT2,
    };
};