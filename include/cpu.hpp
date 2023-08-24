#ifndef CPU_H
#define CPU_H
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include "bytedefs.hpp"
#include "exceptions.hpp"
#include "memory.hpp"
#include "printer.hpp"

namespace emulator {

struct fetch_result {
  u8 opcode;
  u32 masked_instruction;

  fetch_result(u8 opcode, u32 masked_instruction)
      : opcode(opcode), masked_instruction(masked_instruction) {}
};

struct cpu {
  friend struct cpu_breaker;

  struct opcodes {
    /* 0x01 */ static constexpr u8 MOVE = 0x01;
    /* 0x02 */ static constexpr u8 AND_R = 0x02;
    /* 0x03 */ static constexpr u8 OR_R = 0x03;
    /* 0x04 */ static constexpr u8 NOT_R = 0x04;
    /* 0x05 */ static constexpr u8 XOR_R = 0x05;
    /* 0x06 */ static constexpr u8 LLSH_R = 0x06;
    /* 0x07 */ static constexpr u8 ALSH_R = 0x07;
    /* 0x08 */ static constexpr u8 LRSH_R = 0x08;
    /* 0x09 */ static constexpr u8 ARSH_R = 0x09;
    /* 0x10 */ static constexpr u8 HALT = 0x10;
    /* 0x12 */ static constexpr u8 AND_I = 0x12;
    /* 0x13 */ static constexpr u8 OR_I = 0x13;
    /* 0x14 */ static constexpr u8 NOT_I = 0x14;
    /* 0x15 */ static constexpr u8 XOR_I = 0x15;
    /* 0x16 */ static constexpr u8 LLSH_I = 0x16;
    /* 0x17 */ static constexpr u8 ALSH_I = 0x17;
    /* 0x18 */ static constexpr u8 LRSH_I = 0x18;
    /* 0x19 */ static constexpr u8 ARSH_I = 0x19;
    /* 0x21 */ static constexpr u8 LOAD_AT_ADDR = 0x21;
    /* 0x22 */ static constexpr u8 STORE_AT_ADDR = 0x22;
    /* 0x30 */ static constexpr u8 RET = 0x30;
    /* 0x40 */ static constexpr u8 CALL_FN_I = 0x40;
    /* 0x61 */ static constexpr u8 INC_A = 0x61;
    /* 0x62 */ static constexpr u8 INC_B = 0x62;
    /* 0x63 */ static constexpr u8 INC_X = 0x63;
    /* 0x81 */ static constexpr u8 ADD_DSS = 0x81;
    /* 0x82 */ static constexpr u8 SUB_DSS = 0x82;
    /* 0x84 */ static constexpr u8 MULT_DSS = 0x84;
    /* 0x91 */ static constexpr u8 ADD_DSI = 0x91;
    /* 0x92 */ static constexpr u8 SUB_DSI = 0x92;
    /* 0x94 */ static constexpr u8 MULT_DSI = 0x94;
    /* 0x9A */ static constexpr u8 SQRT_R_I = 0x9A;
    /* 0xA5 */ static constexpr u8 LD_IM_A = 0xA5;
    /* 0xA6 */ static constexpr u8 LD_IM_B = 0xA6;
    /* 0xA7 */ static constexpr u8 LD_IM_X = 0xA7;
    /* 0xB1 */ static constexpr u8 TEST_EQ = 0xB1;
    /* 0xB2 */ static constexpr u8 TEST_NEQ = 0xB2;
    /* 0xB3 */ static constexpr u8 TEST_CTRL_NEG = 0xB3;
    /* 0xC1 */ static constexpr u8 PRINT_I_R = 0xC1;
    /* 0xCC */ static constexpr u8 PUTC_R = 0xCC;
    /* 0xCF */ static constexpr u8 GETC_R = 0xCF;
    /* 0xD1 */ static constexpr u8 REG_PUSH = 0xD1;
    /* 0xD2 */ static constexpr u8 REG_POP = 0xD2;
    /* 0xD9 */ static constexpr u8 POPCNT = 0xD9;
    /* 0xE1 */ static constexpr u8 JMP_WITH_OFFSET = 0xE1;
    /* 0xE2 */ static constexpr u8 JMP = 0xE2;
    /* 0xEA */ static constexpr u8 BNCH_WITH_OFFSET = 0xEA;
    /* 0xEE */ static constexpr u8 BNCH = 0xEE;
    /* 0xEB */ static constexpr u8 RND_SEED = 0xEB;
    /* 0xEC */ static constexpr u8 RND_NUM = 0xEC;
    /* 0xF0 */ static constexpr u8 EXT_INSTR = 0xF0;
  };

  struct extended_opcodes {
    /* 0x10 */ static constexpr u8 LOAD_FIM_FA = 0x10;
    /* 0x10 */ static constexpr u8 LOAD_FIM_FB = 0x11;
    /* 0x81 */ static constexpr u8 FADD_DSS = 0x81;
    /* 0x82 */ static constexpr u8 FSUB_DSS = 0x82;
    /* 0x84 */ static constexpr u8 FMULT_DSS = 0x84;
    /* 0x91 */ static constexpr u8 FADD_DSI = 0x91;
    /* 0x92 */ static constexpr u8 FSUB_DSI = 0x92;
    /* 0x94 */ static constexpr u8 FMULT_DSI = 0x94;
    /* 0xA2 */ static constexpr u8 FSQRT_R_I = 0xA2;
  };

  struct ctrl_bits {
    static constexpr u32 CTRL_ZERO_BIT = 0x00000001 << 0;
    static constexpr u32 CTRL_CARRY_BIT = 0x00000001 << 1;
    static constexpr u32 CTRL_NEG_BIT = 0x00000001 << 2;
    static constexpr u32 CTRL_TEST_TRUE = 0x00000001 << 3;
    static constexpr u32 CTRL_EXT_FNC = 0x80000000;
  };

  cpu();

  void
  reset();

  int
  cycles() const noexcept;

  bool
  is_halted() const noexcept;

  void
  dump_registers(printer out = metaout) const;

  void
  run();

  void
  tick();

  void
  set_memory(byte const* bytes, u64 count, u64 addr_start);

 private:
  [[noreturn]] static void
  invalid_registers();

  bool halted = false;

  // Registers

  // GP registers
  u32 a, b, x;

  // FP registers
  f64 fa, fb, fx;

  // permentantly zero register good for TEST instructions
  u32 const z = 0;

  // special
  u32 sp, ra, pc;

  // ctrl - for flag bits
  u32 ctrl;

  // ram
  memory<u8, 128, 512, u32> ram;

  // convenience access for decoding instructions
  std::vector<u32*> const regs = {{(u32*)&z, &a, &b, &x, &sp, &ra}};
  std::vector<f64*> const fregs = {{(f64*)&z, &fa, &fb, &fx}};

  int m_cycles = 0;

  void
  zero_check() const noexcept;

  void
  ctrl_set(u32 bitmask);

  [[nodiscard]] u32
  ctrl_get(u32 bitmask) const;

  void
  ctrl_clear(u32 setbits);

  void
  reg_store(u32 reg, u32 start_addr);

  [[nodiscard]] u32
  fetch(u32 const& r) const;

  fetch_result
  get_next_instruction();

  void
  set_needed_ctrl(u32* regptr);

  void
  execute_instruction(u8 opcode, u32 instruction);

  void
  execute_extended_instruction(u8 opcode, u32 instruction);

  template <typename RegType>
  [[nodiscard]] std::tuple<RegType*, RegType*, RegType*>
  register_decode_dss(u32 instruction) const {
    u32 ssb = instruction & 0xff;
    u32 srb = (instruction & 0xff00) >> 8;
    u32 sdb = (instruction & 0xff0000) >> 16;
    RegType* des = register_decode<RegType>(sdb);
    RegType* sr = register_decode<RegType>(srb);
    RegType* sc = register_decode<RegType>(ssb);
    return std::make_tuple(des, sr, sc);
  }

  template <typename RegType>
  [[nodiscard]] std::pair<RegType*, RegType*>
  register_decode_dsi(u32 instruction) const {
    u32 sdb = (instruction & 0xff0000) >> 16;
    u32 srb = (instruction & 0xff00) >> 8;

    RegType* des = register_decode<RegType>(sdb);
    RegType* sr = register_decode<RegType>(srb);
    return std::make_pair(des, sr);
  }

  template <typename RegType>
  [[nodiscard]] std::pair<RegType*, RegType*>
  register_decode_both(u32 instruction) const {
    u32 reg1 = instruction & 0xff;
    u32 reg2 = (instruction & 0xff00) >> 8;
    metaout << "reg1 " << reg1 << "reg2 " << reg2 << " and " << std::hex
            << instruction << endl;
    RegType* lhs = register_decode<RegType>(reg1);
    RegType* rhs = register_decode<RegType>(reg2);
    return std::make_pair(lhs, rhs);
  }

  template <typename RegType>
  [[nodiscard]] RegType*
  register_decode_first(u32 instruction) const {
    u32 reg1 = instruction & 0xff;
    return register_decode<RegType>(reg1);
  }

  template <typename RegType>
  [[nodiscard]] RegType*
  register_decode(u32 reg_index) const {
    if constexpr (std::is_floating_point_v<RegType>) {
      if (reg_index > fregs.size()) invalid_registers();
      return fregs[reg_index];
    } else {
      if (reg_index > regs.size()) invalid_registers();
      return regs[reg_index];
    }
  }

  template <u64 N, typename Return = u32>
  [[nodiscard]] Return
  literal_decode(u32 instruction) const noexcept {
    u64 mask = (N == (sizeof(u64) * 8)) ? ~((u64)0) : (1llu << N) - 1llu;
    u64 temporary = (instruction & mask);
    Return value = *(Return*)&temporary;
    metaout << mask << " " << temporary << " " << value << endl;
    return value;
  }
};

}  // namespace emulator

#endif
