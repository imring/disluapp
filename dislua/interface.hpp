#pragma once
#include "const.hpp"
#include "buffer.hpp"
#include "trains.hpp"

#include <string>
#include <vector>
#include <any>
#include <sstream>
#include <iomanip>
#include <variant>
#include <complex>
#include <unordered_map>

namespace dislua {
    class interface {
    public:
        struct proto;

        using table_val_t = std::variant<std::nullptr_t, bool, long, double, std::string>;
        using table_t     = std::unordered_map<table_val_t, table_val_t>;

        using kgc_t = std::variant<proto, table_t, long long, unsigned long long,
            std::complex<double>, std::string>;

        struct instruction {
            size_t pos = 0;
            byte   opcode = 0;

            // fields
            byte a = 0;
            union {
                struct { byte c, b; };
                word d = 0;
            };
        };

        struct varname {
            byte        type = 0;
            std::string name;
            size_t      start = 0, end = 0;
        };

        struct proto {
            size_t  start = 0, len = 0;
            byte    flags = 0, numparams = 0, framesize = 0;
            uleb128 firstline = 0, numline = 0;

            std::vector<instruction> ins;
            std::vector<word>        uv;
            std::vector<kgc_t>       kgc;
            std::vector<double>      knum;

            std::vector<dword>       lineinfo;
            std::vector<std::string> uv_names;
            std::vector<varname>     varnames;

            const std::string get_name() const {
                std::stringstream ss;
                ss << "proto:" << std::setfill('0') << std::setw(8) << std::hex << start << '-' << std::setw(8) << start + len;
                return ss.str();
            }
        };

        interface() { reset(); }
        interface(buffer& buf): buf(buf) { reset(); }

        virtual void read()  = 0;
        virtual void write() = 0;
        
        void reset() {
            version_compiler = COMPILER_UNKNOWN;

            header.flags = 0;
            header.debug_name.clear();

            protos.clear();
            buf.reset_index();
        }

        struct {
            uleb128     flags = 0;
            std::string debug_name;
        } header;

        std::vector<proto> protos;
    protected:
        std::vector<proto> _protos;
    public:

        COMPILERS version_compiler = COMPILER_UNKNOWN;
        buffer    buf;
    };
};