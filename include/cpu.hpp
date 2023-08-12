#include "bytedefs.hpp"
#include "memory.hpp"
#include <cinttypes>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace emulator {

static auto const metaout = &std::cerr;
static auto const cpuout = &std::cout;

struct cpu {
  struct opcodes {
    static constexpr u8 LD_IM_A = 0xA5;
    static constexpr u8 LD_IM_B = 0xA6;
    static constexpr u8 INC_A = 0x65;
    static constexpr u8 JMP_WITH_OFFSET = 0x9A;
    static constexpr u8 BNCH_WITH_OFFSET = 0xEA;
    static constexpr u8 HALT = 0x10;
    static constexpr u8 TEST_EQ = 0x99;
    static constexpr u8 TEST_NEQ = 0xa0;
    static constexpr u8 PRINT_I_A = 0xc1;
    static constexpr u8 PUTC_R = 0xcc;
    static constexpr u8 CALL_FN_I = 0x32;
    static constexpr u8 RET = 0x30;
    static constexpr u8 MULT_DSS = 0x84;
    static constexpr u8 ADD_DSI = 0x91;
    static constexpr u8 SUB_DSI = 0x92;
    static constexpr u8 REG_PUSH = 0xe1;
    static constexpr u8 REG_POP = 0xe2;
  };

  static constexpr u32 CTRL_ZERO_A_BIT = 0x00000001 << 0;
  static constexpr u32 CTRL_CARRY_A_BIT = 0x00000001 << 4;
  static constexpr u32 CTRL_NEG_A_BIT = 0x00000001 << 8;

  static constexpr u32 CTRL_ZERO_B_BIT = 0x00000001 << 1;
  static constexpr u32 CTRL_CARRY_B_BIT = 0x00000001 << 5;
  static constexpr u32 CTRL_NEG_B_BIT = 0x00000001 << 9;

  static constexpr u32 CTRL_TEST_TRUE = 0x00000001 << 12;

  bool is_halt = false;
  cpu() { reset(); }
  void reset() {
    is_halt = false;
    pc = 0xF000;
    a = b = x = 0;
    sp = 0x0100;
    fp = sp;
    ctrl = 0;
  }

private:
  void ctrl_set(u32 bitmask) { ctrl |= bitmask; }
  u32 ctrl_get(u32 bitmask) { return ctrl &= bitmask; }
  void ctrl_clear(u32 setbits) { ctrl &= ~(setbits); }

  // Registers
  u32 a, b, x;

  u32 sp, fp, pc;

  u32 ctrl;

  memory<u8, 65536, u32> ram;

  std::tuple<u32 *, u32 *, u32 *> register_decode_dss(u32 instruction) {
    u32 ssb = instruction & 0xff;
    u32 srb = (instruction & 0xff00) >> 8;
    u32 sdb = (instruction & 0xff0000) >> 16;
    u32 *des = register_decode(sdb);
    u32 *sr = register_decode(srb);
    u32 *sc = register_decode(ssb);
    return std::make_tuple(des, sr, sc);
  }

  std::pair<u32 *, u32 *> register_decode_dsi(u32 instruction) {
    u32 sdb = (instruction & 0xff0000) >> 16;
    u32 srb = (instruction & 0xff00) >> 8;
    u32 *des = register_decode(sdb);
    u32 *sr = register_decode(srb);
    return std::make_pair(des, sr);
  }

  std::pair<u32 *, u32 *> register_decode_both(u32 instruction) {
    u32 reg1 = instruction & 0xff;
    u32 reg2 = (instruction & 0xff00) >> 8;
    u32 *lhs = register_decode(reg1);
    u32 *rhs = register_decode(reg2);
    return std::make_pair(lhs, rhs);
  }

  u32 *register_decode_first(u32 instruction) {
    u32 reg1 = instruction & 0xff;
    return register_decode(reg1);
  }

  u32 *register_decode(u32 reg_index) {
    static const std::vector<u32 *> regs = {{NULL, &a, &b, &x, &sp, &fp}};
    if (reg_index < 1 || reg_index > regs.size())
      malformed();
    return regs[reg_index];
  }

  u32 fetch(u32 const &r) {
    u32 instr = (ram[r] << 24) | (ram[r + 1] << 16) | (ram[r + 2] << 8) |
                (ram[r + 3] << 0);
    return instr;
  }

  void ctrl_a_check() {
    if (a > 8388607u) {
      a = -(16777216u - a);
      ctrl_set(CTRL_NEG_A_BIT);
    } else if (a == 0) {
      ctrl_clear(CTRL_NEG_A_BIT);
      ctrl_set(CTRL_ZERO_A_BIT);
    } else {
      ctrl_clear(CTRL_NEG_A_BIT);
      ctrl_clear(CTRL_ZERO_A_BIT);
    }
  }
  void ctrl_b_check() {
    if (b > 8388607u) {
      b = -(16777216u - b);
      ctrl_set(CTRL_NEG_B_BIT);
    } else if (b == 0) {
      ctrl_clear(CTRL_NEG_B_BIT);
      ctrl_set(CTRL_ZERO_B_BIT);
    } else {
      ctrl_clear(CTRL_NEG_B_BIT);
      ctrl_clear(CTRL_ZERO_B_BIT);
    }
  }

  u32 literal_decode(u32 instruction) {
    u32 literal = instruction & 0xffffff;
    return literal;
  }

  [[noreturn]] void malformed() {
    throw std::invalid_argument(
        "The CPU attempted to execute a malformed instruction");
  };

  void reg_store(u32 reg, u32 start_addr) {
    ram[start_addr + 3] = (reg & 0xff);
    ram[start_addr + 2] = (reg & 0xff00) >> 8;
    ram[start_addr + 1] = (reg & 0xff0000) >> 16;
    ram[start_addr + 0] = (reg & 0xff00000) >> 24;
  }

public:
  void run() {
    while (!is_halt)
      tick();
  }

  void tick() {
    *metaout << "tick" << std::endl;
    if (is_halt)
      return;

    u32 instruction = fetch(pc);
    pc += 4;
    byte opcode = instruction >> 24;

    switch (opcode) {
    case opcodes::LD_IM_A: {
      *metaout << "Loading into a " << literal_decode(instruction) << std::endl;
      a = literal_decode(instruction);
      ctrl_a_check();
    } break;
    case opcodes::LD_IM_B: {
      *metaout << "Loading into b " << literal_decode(instruction) << std::endl;
      b = literal_decode(instruction);
      ctrl_b_check();
    } break;
    case opcodes::INC_A: {
      a++;
      ctrl_a_check();
      *metaout << "Incrementing A; now " << a << std::endl;
    } break;
    case opcodes::BNCH_WITH_OFFSET: {
      *metaout << "Conditional branch... " << std::endl;

      if (!ctrl_get(CTRL_TEST_TRUE)) {
        *metaout << "... not taken " << std::endl;
        break;
      }
    }
    case opcodes::JMP_WITH_OFFSET: {
      *metaout << " JUMP " << std::endl;
      u32 addr = literal_decode(instruction);
      pc -= addr;
      *metaout << "pc is now at " << addr << " and is " << pc << std::endl;
    } break;
    case opcodes::HALT: {
      is_halt = true;
      *metaout << "Halting." << std::endl;
    } break;
    case opcodes::TEST_EQ:
    case opcodes::TEST_NEQ: {
      auto predicate = [&opcode](auto l, auto r) {
        if (opcode == opcodes::TEST_EQ)
          return l == r;
        else if (opcode == opcodes::TEST_NEQ)
          return l != r;
        else
          throw std::invalid_argument("invalid opcode");
      };

      *metaout << "testing " << std::endl;
      auto [lhs, rhs] = register_decode_both(instruction);
      if (predicate(*lhs, *rhs)) {
        ctrl_set(CTRL_TEST_TRUE);
      } else {
        ctrl_clear(CTRL_TEST_TRUE);
      }

    } break;
    case opcodes::PRINT_I_A: {
      *cpuout << a;
    } break;
    case opcodes::PUTC_R: {
      auto *reg = register_decode_first(instruction);
      *cpuout << static_cast<char>(*reg);
    } break;
    case opcodes::CALL_FN_I: {
      *metaout << "Calling function "
               << "storing pc " << pc << " into " << static_cast<int>(ram[sp])
               << " from sp of " << sp << std::endl;
      u32 addr = literal_decode(instruction);
      reg_store(pc, sp);
      sp += 4;
      fp = sp;
      pc = addr;
    } break;
    case opcodes::RET: {
      u32 addr = fetch(fp - 4);
      *metaout << "Returning to address at " << fp - 4 << " which is " << addr
               << std::endl;
      fp -= 4;
      sp = fp;
      pc = addr;
    } break;
    case opcodes::ADD_DSI:
    case opcodes::SUB_DSI: {
      auto operation = [&opcode](auto a, auto b) {
        if (opcode == opcodes::ADD_DSI)
          return a + b;
        else
          return a - b;
      };
      auto [dest, l] = register_decode_dsi(instruction);
      auto short_literal = instruction & 0xff;
      *metaout << "Setting " << *dest << " to " << *l << " op " << short_literal
               << std::endl;
      *dest = operation(*l, short_literal);
    } break;
    case opcodes::MULT_DSS: {
      auto [dest, l, r] = register_decode_dss(instruction);
      *metaout << "multiplying " << *l << " * " << *r << std::endl;
      *dest = *l * *r;
    } break;
    case opcodes::REG_PUSH: {
      *metaout << "pushing to addr " << sp << std::endl;
      reg_store(*register_decode_first(instruction), sp);
      sp += 4;
    } break;
    case opcodes::REG_POP: {
      auto reg = register_decode_first(instruction);
      *reg = fetch(sp - 4);
      *metaout << "popping got value " << *reg << " from addr " << (sp - 4)
               << std::endl;
      sp -= 4;
    } break;
    default: {
      std::stringstream msg;
      msg << "No such opcode " << static_cast<int>(opcode) << " in instruction "
          << instruction;
      throw std::invalid_argument(msg.str());
    }
    }
  }

  void set_memory(byte *bytes, u64 count, u64 addr_start) {
    ram.check_addr(addr_start + count);
    for (int i = 0; i < count; i++) {
      ram[addr_start + i] = bytes[i];
    }
  }
};
} // namespace emulator
