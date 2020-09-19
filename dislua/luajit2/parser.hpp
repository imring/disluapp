#pragma once
#include "../interface.hpp"
#include "../buffer.hpp"
#include "../trains.hpp"
#include "const.hpp"

#include <iostream>
#include <map>
#include <complex>
#include <algorithm>
#include <cmath>

namespace dislua {
	namespace lj2 {
		class parser: public interface {
		public:
			parser(interface &info): interface(info) {}
			parser(buffer &_buf): interface(_buf) {}
			parser(): interface() {}

			void read() {
				//throw std::runtime_error("test");

				read_header();
				while (!buf.index_better_size()) {
					if (!buf.read(false)) break;

					uleb128 len = buf.read_uleb128();
					if (!len) throw std::runtime_error("LuaJIT 2: Prototype size == 0.");

					proto pt = {0};
					//std::cout << len << '\n';
					read_proto(pt);
					//std::cout << len << ' ' << pt.len << '\n';
					if (pt.len != len) throw std::runtime_error("LuaJIT 2: Different prototype sizes.");
					_protos.push_back(pt);
					protos.push_back(pt);
				}

				version_compiler = COMPILER_LUAJIT2;
			}

			void write() {
				//std::cout << '\n';
				buf.clear();

				write_header();
				for (proto &pt: protos) write_proto(pt);
				buf.write<byte>(0);
			}

		private:
			void read_header() {
				byte head = 0;
				if ((buf.read() != header::HEAD1) ||
					(buf.read() != header::HEAD2) ||
					(buf.read() != header::HEAD3) ||

					(buf.read() != header::VERSION))
					throw std::runtime_error("LuaJIT 2: Invalid header.");

				uleb128 flags = buf.read_uleb128();
				header.flags = flags;

				flags &= DUMP_BE;
				flags &= DUMP_STRIP;
				flags &= DUMP_FFI;
				flags &= DUMP_FR2;
				if (flags) throw std::runtime_error("LuaJIT 2: Unknown header flags.");

				if (!(header.flags & DUMP_STRIP)) {
					uleb128 len = buf.read_uleb128();
					header.debug_name.resize(len);
					buf.read(header.debug_name.begin(), header.debug_name.end());
				}
			}

			void read_bc_instructions(proto &pt, uleb128 size) {
				while (size) {
					dword ins = buf.read<dword>();

					instruction cins;
					cins.pos = buf.get_index() - 4;
					cins.opcode = ins & 0xff;
					cins.a = (ins >> 8) & 0xff;
					cins.d = ins >> 16;

					pt.ins.push_back(cins);
					--size;
				}
			}

			void read_uv(proto &pt, byte size) {
				while (size) {
					word uv_data = buf.read<word>();
					pt.uv.push_back(uv_data);
					--size;
				}
			}

			table_val_t read_ktabk() {
				uleb128 type = buf.read_uleb128();
				table_val_t value;

				if (type >= KTAB_STR) {
					std::string v; v.resize(type - KTAB_STR);
					buf.read(v.begin(), v.end());
					value = v;
				} else if (type == KTAB_NUM) {
					uleb128 vals[2] = { 0 };
					buf.read_uleb128(vals, 2);
					value = *detail::pointer_cast<double*>(vals);
				} else if (type == KTAB_INT) {
					uleb128 v = buf.read_uleb128();
					value = static_cast<long>(v);
				} else if (type == KTAB_NIL) value = nullptr;
				else value = type == KTAB_TRUE;

				return value;
			}

			table_t read_ktab() {
				table_t table;
				uleb128 narray = buf.read_uleb128();
				uleb128 nhash = buf.read_uleb128();
				// std::cout << narray << ' ' << nhash << '\n';

				for (uleb128 i = 0; i < narray; ++i) {
					table_val_t value = read_ktabk();
					table.insert({ long(i), value });
				}
				while (nhash) {
					table_val_t key = read_ktabk();
					table_val_t value = read_ktabk();
					table.insert({ key, value });
					--nhash;
				}

				return table;
			}

			void read_kgc(proto &pt, uleb128 size) {
				uleb128 type = KGC_CHILD;
				while (size) {
					type = buf.read_uleb128();

					if (type == KGC_CHILD) {
						pt.kgc.push_back(_protos.back());
						_protos.pop_back();
					} else if (type == KGC_TAB) {
						table_t table = read_ktab();
						pt.kgc.push_back(table);
					} else if (type == KGC_I64) {
						long long v = 0;
						buf.read_uleb128(detail::pointer_cast<uleb128 *>(&v), sizeof(long long) / sizeof(uleb128));
						pt.kgc.push_back(v);
					} else if (type == KGC_U64) {
						unsigned long long v = 0;
						buf.read_uleb128(detail::pointer_cast<uleb128 *>(&v), sizeof(unsigned long long) / sizeof(uleb128));
						pt.kgc.push_back(v);
					} else if (type == KGC_COMPLEX) {
						std::complex<double> v;
						buf.read_uleb128(detail::pointer_cast<uleb128 *>(&v), sizeof(std::complex<double>) / sizeof(uleb128));
						pt.kgc.push_back(v);
					} else if (type >= KGC_STR) {
						dword length = type - KGC_STR;
						std::string str; str.resize(length);
						buf.read(str.begin(), str.end());
						pt.kgc.push_back(str);
					}

					--size;
				}
			}

			void read_knum(proto &pt, uleb128 size) {
				while (size) {
					bool isnum = buf.read(false) & 1;

					uleb128 result[2] = { buf.read_uleb128_33(), 0 };
					double v = 0;
					if (isnum) {
						result[1] = buf.read_uleb128();
						v = *detail::pointer_cast<double *>(result);
					} else v = double(static_cast<long>(result[0]));
					pt.knum.push_back(v);
					--size;
				}
			}

			void read_lineinfo(proto &pt, uleb128 size) {
				while (size) {
					dword line = 0;
					if (pt.numline >= 1 << 16) line = buf.read<dword>();
					else if (pt.numline >= 1 << 8) line = dword(buf.read<word>());
					else line = dword(buf.read());
					pt.lineinfo.push_back(pt.firstline + line);
					--size;
				}
			}

			void read_uvname(proto &pt, byte size) {
				while (size) {
					std::string name;
					buf.read_to_zero(name);
					pt.uv_names.push_back(name);
					--size;
				}
			}

			void read_varname(proto &pt) {
				size_t last = 0;
				while (true) {
					byte type = buf.read();
					if (type == VARNAME_END) break;

					varname info = { 0 };
					info.type = type;

					switch (type) {
					case VARNAME_FOR_IDX:   info.name = "(index)"; break;
					case VARNAME_FOR_STOP:  info.name = "(limit)"; break;
					case VARNAME_FOR_STEP:  info.name = "(step)"; break;
					case VARNAME_FOR_GEN:   info.name = "(generator)"; break;
					case VARNAME_FOR_STATE: info.name = "(state)"; break;
					case VARNAME_FOR_CTL:   info.name = "(control)"; break;
					default:
						info.name.push_back(type);
						buf.read_to_zero(info.name);
						break;
					}

					last = info.start = last + size_t(buf.read_uleb128());
					info.end = info.start + size_t(buf.read_uleb128());

					pt.varnames.push_back(info);
				}
			}

			void read_proto(proto &pt) {
				uleb128 sizekgc, sizekn, sizebc;
				byte sizeuv;

				pt.start = buf.get_index();

				pt.flags = buf.read();
				pt.numparams = buf.read();
				pt.framesize = buf.read();
				sizeuv = buf.read();

				sizekgc = buf.read_uleb128();
				sizekn = buf.read_uleb128();
				sizebc = buf.read_uleb128();

				byte flags = pt.flags;
				flags &= PROTO_CHILD;
				flags &= PROTO_VARARG;
				flags &= PROTO_FFI;
				flags &= PROTO_NOJIT;
				flags &= PROTO_ILOOP;
				if (flags) throw std::runtime_error("LuaJIT 2: Unknown prototype flags.");

				uleb128 sizedbg = 0;
				if (!(header.flags & DUMP_STRIP)) {
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

				size_t sizedbg_read = buf.get_index();
				if (sizedbg) {
					read_lineinfo(pt, sizebc);
					read_uvname(pt, sizeuv);
					read_varname(pt);
				}
				sizedbg_read = buf.get_index() - sizedbg_read;
				if (sizedbg_read != sizedbg) throw std::runtime_error("LuaJIT 2: Invalid debuginfo size.");

				pt.len = buf.get_index() - pt.start;
			}

			void write_header() {
				buf.write(header::HEAD1);
				buf.write(header::HEAD2);
				buf.write(header::HEAD3);

				buf.write(header::VERSION);
				buf.write_uleb128(header.flags);

				if (!(header.flags & DUMP_STRIP)) {
					buf.write_uleb128(uleb128(header.debug_name.size()));
					buf.write(header.debug_name.begin(), header.debug_name.end());
				}
			}

			void write_bc_instructions(proto &pt, buffer &ptbuf) {
				for (instruction &cins: pt.ins) {
					dword ins = cins.opcode | (cins.a << 8) | (cins.d << 16);
					ptbuf.write(ins);
				}
			}

			void write_uv(proto &pt, buffer &ptbuf) {
				for (word &uv: pt.uv)
					ptbuf.write(uv);
			}

			void write_ktabk(const table_val_t &val, buffer &pthash) {
				std::visit(detail::overloaded {
					[&](std::nullptr_t) { pthash.write_uleb128(KTAB_NIL); },
					[&](bool arg) { pthash.write_uleb128(arg ? KTAB_TRUE : KTAB_FALSE); },
					[&](long arg) {
						pthash.write_uleb128(KTAB_INT);
						pthash.write_uleb128(uleb128(arg));
					},
					[&](double arg) {
						pthash.write_uleb128(KTAB_NUM);
						pthash.write_uleb128(detail::pointer_cast<uleb128 *>(&arg), 2);
					},
					[&](std::string arg) {
						pthash.write_uleb128(KTAB_STR + uleb128(arg.size()));
						pthash.write(arg.begin(), arg.end());
					}
				}, val);
			}

			void write_ktab(table_t &table, buffer &ptbuf) {
				buffer ptarray, pthash;
				table_t copy_table = table;

				long i = 0;
				while (true) {
					auto it = copy_table.find(i++);
					if (it == copy_table.end()) break;
					write_ktabk(it->second, ptarray);
					copy_table.erase(it->first);
				}

				for (auto &kv: copy_table) {
					write_ktabk(kv.first, pthash);
					write_ktabk(kv.second, pthash);
				}

				ptbuf.write_uleb128(uleb128(i - 1));
				ptbuf.write_uleb128(uleb128(copy_table.size()));
				ptbuf.write(ptarray);
				ptbuf.write(pthash);
			}

			void write_kgc(proto &pt, buffer &ptbuf) {
				for (kgc_t &kgc: pt.kgc)
					std::visit(detail::overloaded {
						[&](proto) { ptbuf.write_uleb128(KGC_CHILD); },
						[&](table_t arg) {
							ptbuf.write_uleb128(KGC_TAB);
							write_ktab(arg, ptbuf);
						},
						[&](long long arg) {
							ptbuf.write_uleb128(KGC_I64);
							ptbuf.write_uleb128(detail::pointer_cast<uleb128 *>(&arg), sizeof(long long) / sizeof(uleb128));
						},
						[&](unsigned long long arg) {
							ptbuf.write_uleb128(KGC_U64);
							ptbuf.write_uleb128(detail::pointer_cast<uleb128 *>(&arg), sizeof(unsigned long long) / sizeof(uleb128));
						},
						[&](std::complex<double> arg) {
							ptbuf.write_uleb128(KGC_COMPLEX);
							ptbuf.write_uleb128(detail::pointer_cast<uleb128 *>(&arg), sizeof(std::complex<double>) / sizeof(uleb128));
						},
						[&](std::string arg) {
							ptbuf.write_uleb128(uleb128(arg.size()) + KGC_STR);
							ptbuf.write(arg.begin(), arg.end());
						}
					}, kgc);
			}

			void write_knum(proto &pt, buffer &ptbuf) {
				for (double &val: pt.knum) {
					//std::cout << std::hex << buf.get_index() + ptbuf.get_index() << std::dec << '\n';
					if (!std::fmod(val, 1)) ptbuf.write_uleb128_33(uleb128(val));
					else {
						uleb128 *v = detail::pointer_cast<uleb128 *>(&val);
						ptbuf.write_uleb128_33(v[0], true);
						ptbuf.write_uleb128(v[1]);
					}
				}
			}

			void write_lineinfo(proto &pt, buffer &ptdebug) {
				if (pt.lineinfo.size() != pt.ins.size()) throw std::runtime_error("LuaJIT 2: Lines count != instructions count");
				size_t size = pt.ins.end() - pt.ins.begin();
				for(dword line: pt.lineinfo) {
					if (pt.numline >= 1 << 16) ptdebug.write(line - pt.firstline);
					else if (pt.numline >= 1 << 8) ptdebug.write(word(line - pt.firstline));
					else ptdebug.write(byte(line - pt.firstline));
					--size;
				}
			}

			void write_uvname(proto &pt, buffer &ptdebug) {
				if (pt.lineinfo.size() != pt.ins.size()) throw std::runtime_error("LuaJIT 2: Count of upvalue names != size of BC instructions");
				for (std::string &name: pt.uv_names) {
					ptdebug.write(name.begin(), name.end());
					ptdebug.write<byte>(0);
				}
			}

			void write_varname(proto &pt, buffer &ptdebug) {
				size_t last = 0;
				for (varname &info: pt.varnames) {
					if (info.type >= VARNAME__MAX) {
						ptdebug.write(info.name.begin(), info.name.end());
						ptdebug.write<byte>(0);
					} else ptdebug.write(info.type);
					ptdebug.write_uleb128(uleb128(info.start - last));
					last = info.start;
					ptdebug.write_uleb128(uleb128(info.end - info.start));
				}
				ptdebug.write(VARNAME_END);
			}

			void write_proto(proto &pt) {
				buffer ptbuf, ptdebug;
				size_t sizedebug = 0;

				pt.start = buf.get_index();

				ptbuf.write(pt.flags);
				ptbuf.write(pt.numparams);
				ptbuf.write(pt.framesize);
				ptbuf.write(byte(pt.uv.size()));

				ptbuf.write_uleb128(uleb128(pt.kgc.size()));
				ptbuf.write_uleb128(uleb128(pt.knum.size()));
				ptbuf.write_uleb128(uleb128(pt.ins.size()));

				if (!(header.flags & DUMP_STRIP)) {
					write_lineinfo(pt, ptdebug);
					write_uvname(pt, ptdebug);
					write_varname(pt, ptdebug);

					sizedebug = ptdebug.get_index();
					ptbuf.write_uleb128(uleb128(sizedebug));
					if (sizedebug) {
						ptbuf.write_uleb128(pt.firstline);
						ptbuf.write_uleb128(pt.numline);
					}
				}

				write_bc_instructions(pt, ptbuf);
				write_uv(pt, ptbuf);
				write_kgc(pt, ptbuf);
				write_knum(pt, ptbuf);

				if (sizedebug) ptbuf.write(ptdebug);

				//std::cout << ptbuf.get_index() << '\n';
				pt.len = ptbuf.get_index();
				buf.write_uleb128(uleb128(pt.len));
				buf.write(ptbuf);

				/*pt.start = buf.get_index();

				pt.flags = buf.read();
				pt.numparams = buf.read();
				pt.framesize = buf.read();
				pt.sizeuv = buf.read();

				pt.sizekgc = buf.read_uleb128();
				pt.sizekn = buf.read_uleb128();
				pt.sizebc = buf.read_uleb128();

				byte flags = pt.flags;
				flags &= PROTO_CHILD;
				flags &= PROTO_VARARG;
				flags &= PROTO_FFI;
				flags &= PROTO_NOJIT;
				flags &= PROTO_ILOOP;
				if (flags) throw std::runtime_error("LuaJIT 2: Unknown prototype flags.");

				uleb128 sizedbg = 0;
				if (!(header.flags & DUMP_STRIP)) {
					sizedbg = buf.read_uleb128();
					if (sizedbg) {
						pt.firstline = buf.read_uleb128();
						pt.numline = buf.read_uleb128();
					}
				}

				read_bc_instructions(pt);
				read_uv(pt);
				read_kgc(pt);
				read_knum(pt);

				size_t sizedbg_read = buf.get_index();
				if (sizedbg) {
					read_lineinfo(pt);
					read_uvname(pt);
					read_varname(pt);
				}
				sizedbg_read = buf.get_index() - sizedbg_read;
				if (sizedbg_read != sizedbg) throw std::runtime_error("LuaJIT 2: Invalid debuginfo size.");

				pt.len = buf.get_index() - pt.start;*/
			}
		};
	}
};