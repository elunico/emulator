#include <cinttypes>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "bytedefs.hpp"
#include "exceptions.hpp"
#include "memory.hpp"
#include "printer.hpp"

namespace emulator {

static printer metaout = printer{};  // &std::cerr;
static printer cpuout = &std::cout;

struct cpu {
  struct opcodes {
    /* 0x01 */ static constexpr u8 MOVE = 0x01;
    /* 0x02 */ static constexpr u8 AND_R = 0x02;
    /* 0x03 */ static constexpr u8 OR_R = 0x03;
    /* 0x04 */ static constexpr u8 NOT_R = 0x04;
    /* 0x05 */ static constexpr u8 XOR_R = 0x05;
    /* 0x12 */ static constexpr u8 AND_I = 0x12;
    /* 0x13 */ static constexpr u8 OR_I = 0x13;
    /* 0x14 */ static constexpr u8 NOT_I = 0x14;
    /* 0x15 */ static constexpr u8 XOR_I = 0x15;
    /* 0x10 */ static constexpr u8 HALT = 0x10;
    /* 0x21 */ static constexpr u8 LOAD_AT_ADDR = 0x21;
    /* 0x30 */ static constexpr u8 RET = 0x30;
    /* 0x40 */ static constexpr u8 CALL_FN_I = 0x40;
    /* 0x61 */ static constexpr u8 INC_A = 0x61;
    /* 0x62 */ static constexpr u8 INC_B = 0x62;
    /* 0x63 */ static constexpr u8 INC_X = 0x63;
    /* 0x81 */ static constexpr u8 ADD_DSS = 0x81;
    /* 0x84 */ static constexpr u8 MULT_DSS = 0x84;
    /* 0x94 */ static constexpr u8 MULT_DSI = 0x94;
    /* 0x91 */ static constexpr u8 ADD_DSI = 0x91;
    /* 0x92 */ static constexpr u8 SUB_DSI = 0x92;
    /* 0x82 */ static constexpr u8 SUB_DSS = 0x82;
    /* 0xA5 */ static constexpr u8 LD_IM_A = 0xA5;
    /* 0xA6 */ static constexpr u8 LD_IM_B = 0xA6;
    /* 0xA7 */ static constexpr u8 LD_IM_X = 0xA7;
    /* 0xB1 */ static constexpr u8 TEST_EQ = 0xB1;
    /* 0xB2 */ static constexpr u8 TEST_NEQ = 0xB2;
    /* 0xC1 */ static constexpr u8 PRINT_I_R = 0xC1;
    /* 0xCC */ static constexpr u8 PUTC_R = 0xCC;
    /* 0xD1 */ static constexpr u8 REG_PUSH = 0xD1;
    /* 0xD2 */ static constexpr u8 REG_POP = 0xD2;
    /* 0xE1 */ static constexpr u8 JMP_WITH_OFFSET = 0xE1;
    /* 0xEA */ static constexpr u8 BNCH_WITH_OFFSET = 0xEA;
    /* 0xF0 */ static constexpr u8 EXT_INSTR = 0xF0;
  };

  struct extended_opcodes {
    /* 0x10 */ static constexpr u8 LOAD_FIM_FA = 0x10;
    /* 0x81 */ static constexpr u8 SQRT_R_I = 0x81;
  };

  struct ctrl_bits {
    static constexpr u32 CTRL_ZERO_BIT = 0x00000001 << 0;
    static constexpr u32 CTRL_CARRY_BIT = 0x00000001 << 1;
    static constexpr u32 CTRL_NEG_BIT = 0x00000001 << 2;
    static constexpr u32 CTRL_TEST_TRUE = 0x00000001 << 3;
    static constexpr u32 CTRL_EXT_FNC = 0x80000000;
  };

  bool is_halt = false;

  cpu() { reset(); }

  void
  reset() {
    is_halt = false;
    pc = 0xF000;
    a = b = x = 0;
    sp = 0x0100;
    ra = 0;
    ctrl = 0;
  }

 private:
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
  memory<u8, 65536, u32> ram;

  // convenience access for decoding instructions
  std::vector<u32*> const regs = {{(u32*)&z, &a, &b, &x, &sp, &ra}};
  std::vector<f64*> const fregs = {{(f64*)&z, &fa, &fb, &fx}};

  void
  ctrl_set(u32 bitmask) {
    ctrl |= bitmask;
  }

  [[nodiscard]] u32
  ctrl_get(u32 bitmask) const {
    return ctrl & bitmask;
  }

  void
  ctrl_clear(u32 setbits) {
    ctrl &= ~(setbits);
  }

  void
  reg_store(u32 reg, u32 start_addr) {
    ram[start_addr + 3] = (reg & 0xff);
    ram[start_addr + 2] = (reg & 0xff00) >> 8;
    ram[start_addr + 1] = (reg & 0xff0000) >> 16;
    ram[start_addr + 0] = (reg & 0xff00000) >> 24;
  }

  [[nodiscard]] u32
  fetch(u32 const& r) const {
    u32 instr = (ram[r] << 24) | (ram[r + 1] << 16) | (ram[r + 2] << 8) |
                (ram[r + 3] << 0);
    return instr;
  }

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

  template <u32 N, typename Return = u32>
  [[nodiscard]] Return
  literal_decode(u32 instruction) const noexcept {
    u32 mask = (1 << N) - 1;
    u32 temporary = (instruction & mask);
    Return value = *(Return*)&temporary;
    return value;
  }

  void
  set_needed_ctrl(u32* regval) {
    if (*regval > 8388607u) {
      *regval = -(16777216u - *regval);
      ctrl_set(ctrl_bits::CTRL_NEG_BIT);
    } else if (*regval == 0) {
      ctrl_clear(ctrl_bits::CTRL_NEG_BIT);
      ctrl_set(ctrl_bits::CTRL_ZERO_BIT);
    } else {
      ctrl_clear(ctrl_bits::CTRL_NEG_BIT);
      ctrl_clear(ctrl_bits::CTRL_ZERO_BIT);
    }
  }

  [[noreturn]] static void
  invalid_registers() {
    throw no_such_register(
        "The CPU attempted to execute a malformed instruction");
  };

  void
  extended_tick() {
    metaout << "extended_tick" << endl;
    if (is_halt) return;

    u32 instruction = fetch(pc);
    pc += 4;
    byte opcode = instruction >> 24;

    switch (opcode) {
      case extended_opcodes::SQRT_R_I: {
        auto* s = register_decode_first<u32>(instruction);
        *s = static_cast<u32>(std::sqrt(*s));
        set_needed_ctrl(s);
      } break;
      case extended_opcodes::LOAD_FIM_FA: {
        f64 lit = literal_decode<24, f64>(instruction);
        fa = lit;
      } break;
      default: {
        throw no_such_opcode("The extended opcode does not exist");
      }
    }

    ctrl_clear(ctrl_bits::CTRL_EXT_FNC);
  }

 public:
  void
  run() {
    while (!is_halt) tick();
  }

  void
  tick() {
    metaout << "tick" << endl;
    if (z != 0) {
      std::cerr << "!*!*!*!*!*!*! The zero register is " << z
                << " !*!*!*!*!*!*!" << endl;
      std::cout
          << "!*!*!*!*!*!*! The zero register has been modified !*!*!*!*!*!*!"
          << endl;
      std::terminate();
    }
    if (is_halt) return;
    if (ctrl_get(ctrl_bits::CTRL_EXT_FNC)) return extended_tick();

    u32 instruction = fetch(pc);
    pc += 4;
    byte opcode = instruction >> 24;

    switch (opcode) {
      case opcodes::AND_R:
      case opcodes::OR_R:
      case opcodes::XOR_R: {
        auto operation = [&opcode](auto a, auto b) {
          if (opcode == opcodes::AND_R)
            return a & b;
          else if (opcode == opcodes::OR_R)
            return a | b;
          else if (opcode == opcodes::XOR_R)
            return a ^ b;
          else
            throw no_such_opcode("opcode does not exist in bit manip group");
        };
        auto [rd, rs, rr] = register_decode_dss<u32>(instruction);
        *rd = operation(*rs, *rr);

      } break;
      case opcodes::AND_I:
      case opcodes::OR_I:
      case opcodes::XOR_I: {
        auto operation = [&opcode](auto a, auto b) {
          if (opcode == opcodes::AND_R)
            return a & b;
          else if (opcode == opcodes::OR_R)
            return a | b;
          else if (opcode == opcodes::XOR_R)
            return a ^ b;
          else
            throw no_such_opcode("opcode does not exist in bit manip group");
        };
        auto [rd, rs] = register_decode_dsi<u32>(instruction);
        auto im = literal_decode<8>(instruction);
        *rd = operation(*rs, im);

      } break;
      case opcodes::MOVE: {
        auto [rd, rs] = register_decode_dsi<u32>(instruction);
        *rd = *rs;
        set_needed_ctrl(rd);
      } break;
      case opcodes::NOT_R: {
        auto [rd, rs] = register_decode_dsi<u32>(instruction);
        *rd = ~*rs;
      } break;
      case opcodes::LOAD_AT_ADDR: {
        metaout << "Loading from address... " << endl;
        auto [rd, rs] = register_decode_dsi<u32>(instruction);
        *rd = ram[*rs];
        metaout << " Loaded value at " << *rs << " is " << *rd << endl;
        set_needed_ctrl(rd);
      } break;
      case opcodes::LD_IM_A: {
        metaout << "Loading into a " << literal_decode<24>(instruction) << endl;
        a = literal_decode<24>(instruction);
        set_needed_ctrl(&a);
      } break;
      case opcodes::LD_IM_B: {
        metaout << "Loading into b " << literal_decode<24>(instruction) << endl;
        b = literal_decode<24>(instruction);
        set_needed_ctrl(&b);
      } break;
      case opcodes::LD_IM_X: {
        metaout << "Loading into b " << literal_decode<24>(instruction) << endl;
        x = literal_decode<24>(instruction);
        set_needed_ctrl(&x);
      } break;
      case opcodes::INC_A: {
        a++;
        set_needed_ctrl(&a);
        metaout << "Incrementing A; now " << a << endl;
      } break;
      case opcodes::INC_B: {
        b++;
        set_needed_ctrl(&b);
        metaout << "Incrementing B; now " << a << endl;
      } break;
      case opcodes::INC_X: {
        x++;
        set_needed_ctrl(&x);
        metaout << "Incrementing X; now " << a << endl;
      } break;
      case opcodes::BNCH_WITH_OFFSET: {
        metaout << "Conditional branch... ";

        if (!ctrl_get(ctrl_bits::CTRL_TEST_TRUE)) {
          metaout << "... not taken " << endl;
          break;
        }
      }
      case opcodes::JMP_WITH_OFFSET: {
        metaout << " JUMP " << endl;
        u32 addr = literal_decode<24>(instruction);
        if (addr & 0x800000) {
          pc += ((~0x800000) & addr);
        } else {
          pc -= addr;
        }
        metaout << "pc is now at " << pc << " and is " << ram[pc] << endl;
      } break;
      case opcodes::HALT: {
        is_halt = true;
        metaout << "Halting." << endl;
      } break;
      case opcodes::TEST_EQ:
      case opcodes::TEST_NEQ: {
        auto predicate = [&opcode](auto l, auto r) {
          if (opcode == opcodes::TEST_EQ)
            return l == r;
          else if (opcode == opcodes::TEST_NEQ)
            return l != r;
          else
            throw no_such_opcode("compared true to TEST once");
        };

        metaout << "testing " << endl;
        auto [lhs, rhs] = register_decode_both<u32>(instruction);
        metaout << "Value lhs = " << *lhs << " and rhs = " << *rhs << endl;
        if (predicate(*lhs, *rhs)) {
          ctrl_set(ctrl_bits::CTRL_TEST_TRUE);
        } else {
          ctrl_clear(ctrl_bits::CTRL_TEST_TRUE);
        }

      } break;
      case opcodes::PRINT_I_R: {
        auto reg = register_decode_first<u32>(instruction);
        cpuout << *reg;
      } break;
      case opcodes::PUTC_R: {
        auto* reg = register_decode_first<u32>(instruction);
        cpuout << static_cast<char>(*reg);
      } break;
      case opcodes::CALL_FN_I: {
        metaout << "Calling function";
        u32 addr = literal_decode<24>(instruction);
        metaout << "Function address is " << addr << endl;
        ra = pc;
        pc = addr;
      } break;
      case opcodes::RET: {
        metaout << "Returning to address at " << ra << endl;
        pc = ra;
      } break;
      case opcodes::ADD_DSI:
      case opcodes::SUB_DSI:
      case opcodes::MULT_DSI: {
        auto operation = [&opcode](auto a, auto b) {
          if (opcode == opcodes::ADD_DSI)
            return a + b;
          else if (opcode == opcodes::SUB_DSI)
            return a - b;
          else if (opcode == opcodes::MULT_DSI)
            return a * b;
          else
            throw no_such_opcode("missing opcode in lambda for DSI");
        };
        auto [dest, l] = register_decode_dsi<u32>(instruction);
        auto short_literal = literal_decode<8>(instruction);
        metaout << "Setting " << *dest << " to " << *l << " op "
                << short_literal << endl;
        *dest = operation(*l, short_literal);
        set_needed_ctrl(dest);
      } break;
      case opcodes::MULT_DSS:
      case opcodes::SUB_DSS:
      case opcodes::ADD_DSS: {
        auto operation = [&opcode](auto a, auto b) {
          if (opcode == opcodes::ADD_DSS)
            return a + b;
          else if (opcode == opcodes::MULT_DSS)
            return a * b;
          else if (opcode == opcodes::SUB_DSS)
            return a - b;
          else
            throw no_such_opcode("missing opcode in lambda for DSS");
        };
        auto [dest, l, r] = register_decode_dss<u32>(instruction);
        metaout << "operating " << *l << " op " << *r << endl;
        // *dest = *l * *r;
        *dest = operation(*l, *r);
        set_needed_ctrl(dest);
      } break;
      case opcodes::REG_PUSH: {
        metaout << "pushing to addr " << sp << endl;
        reg_store(*register_decode_first<u32>(instruction), sp);
        sp += 4;
      } break;
      case opcodes::REG_POP: {
        auto reg = register_decode_first<u32>(instruction);
        *reg = fetch(sp - 4);
        metaout << "popping got value " << *reg << " from addr " << (sp - 4)
                << endl;
        sp -= 4;
      } break;
      case opcodes::EXT_INSTR: {
        ctrl_set(ctrl_bits::CTRL_EXT_FNC);
      } break;
      default: {
        std::stringstream msg;
        msg << "No such opcode " << static_cast<int>(opcode)
            << " in instruction " << instruction;
        throw no_such_opcode(msg.str());
      }
    }
  }

  void
  set_memory(byte const* bytes, u64 count, u64 addr_start) {
    ram.check_addr(addr_start + count);
    for (int i = 0; i < count; i++) {
      ram[addr_start + i] = bytes[i];
    }
  }
};
}  // namespace emulator
