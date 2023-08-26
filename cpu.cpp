#include "cpu.hpp"

#include "bytedefs.hpp"
#include "printer.hpp"

namespace emulator {

u32
get_jump_offset(u32 immediate) {
  if (immediate & 0x800000) {
    return ((~0x800000) & immediate);
  } else {
    return -immediate;
  }
}

// public functions
cpu::cpu() { reset(); }

void
cpu::reset() {
  halted = false;
  pc = 0xF000;
  a = b = x = 0;
  fa = fb = fx = 0.0;
  sp = 0x0100;
  ra = 0;
  ctrl = 0;
  m_cycles = 0;
}

int
cpu::cycles() const noexcept {
  return m_cycles;
}

bool
cpu::is_halted() const noexcept {
  return halted;
}

void
cpu::dump_registers(printer out) const {
  out << std::hex << std::showbase;
  out << "====== Processor Registers ======\n";
  out << "| GP Registers\n";
  out << "|   a = " << a << "  b = " << b << "  x = " << x << "\n";
  out << "| FP Registers\n";
  out << std::dec << std::setprecision(5);
  out << "|   fa = " << fa << "  fb = " << fb << "  fx = " << fx << "\n";
  out << "| Special Registers\n";
  out << std::hex << std::showbase;
  out << "|   sp = " << sp << "  ra = " << ra << "  pc = " << pc << "\n";
  out << "| Zero\n";
  out << "|   z = " << z << "\n";
  out << "| CTRL Register\n";
  out << "|   ctrl = " << ctrl << "\n";
  out << "=================================\n";
  out << std::dec;
}

void
cpu::run() {
  using us = std::chrono::microseconds;
  auto start = std::chrono::system_clock::now();
  while (!halted) tick();
  auto end = std::chrono::system_clock::now();
  auto mseconds = std::chrono::duration_cast<us>(end - start).count();

  metaout << std::dec << "CPU Ran for " << cycles() << " cycles in " << mseconds
          << " us." << endl;
}

void
cpu::tick() {
  m_cycles++;
  metaout << "tick" << endl;
  zero_check();

  if (halted) return;

  auto [opcode, instruction] = get_next_instruction();
  if (ctrl_get(ctrl_bits::CTRL_EXT_FNC)) {
    execute_extended_instruction(opcode, instruction);
  } else {
    execute_instruction(opcode, instruction);
  }
}

void
cpu::set_memory(byte const* bytes, u64 count, u64 addr_start) {
  ram.check_addr(addr_start + count);
  for (int i = 0; i < count; i++) {
    ram[addr_start + i] = bytes[i];
  }
  emulator::metaout << "Loaded " << count << " bytes into memory at "
                    << addr_start << emulator::endl;
}

// private functions

void
cpu::zero_check() const noexcept {
  if (z != 0) {
    std::cerr << "!*!*!*!*!*!*! The zero register is " << z << " !*!*!*!*!*!*!"
              << std::endl;
    std::cout
        << "!*!*!*!*!*!*! The zero register has been modified !*!*!*!*!*!*!"
        << std::endl;
    std::terminate();
  }
}

void
cpu::ctrl_set(u32 bitmask) {
  ctrl |= bitmask;
}

[[nodiscard]] u32
cpu::ctrl_get(u32 bitmask) const {
  return ctrl & bitmask;
}

void
cpu::ctrl_clear(u32 setbits) {
  ctrl &= ~(setbits);
}

void
cpu::reg_store(u32 reg, u32 start_addr) {
  ram[start_addr + 3] = byte_of<0>(reg);
  ram[start_addr + 2] = byte_of<1>(reg);
  ram[start_addr + 1] = byte_of<2>(reg);
  ram[start_addr + 0] = byte_of<3>(reg);
}

[[nodiscard]] u32
cpu::fetch(u32 const& r) const {
  u32 instr = (ram[r] << 24) | (ram[r + 1] << 16) | (ram[r + 2] << 8) |
              (ram[r + 3] << 0);
  return instr;
}

void
cpu::set_needed_ctrl(u32* regptr) {
  if (*regptr > 8388607u) {
    *regptr = -(16777216u - *regptr);
    ctrl_set(ctrl_bits::CTRL_NEG_BIT);
  } else if (*regptr == 0) {
    ctrl_clear(ctrl_bits::CTRL_NEG_BIT);
    ctrl_set(ctrl_bits::CTRL_ZERO_BIT);
  } else {
    ctrl_clear(ctrl_bits::CTRL_NEG_BIT);
    ctrl_clear(ctrl_bits::CTRL_ZERO_BIT);
  }
}

[[noreturn]] void
cpu::invalid_registers() {
  throw no_such_register(
      "The CPU attempted to execute a malformed instruction");
};

fetch_result
cpu::get_next_instruction() {
  u32 instruction = fetch(pc);
  pc += 4;
  u8 opcode = byte_of<3>(instruction);
  return fetch_result(opcode, instruction);
}

void
cpu::execute_instruction(u8 opcode, u32 instruction) {
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
      set_needed_ctrl(rd);
    } break;
    case opcodes::LLSH_R:
    case opcodes::ALSH_R:
    case opcodes::LRSH_R:
    case opcodes::ARSH_R: {
      auto operation = [&opcode](auto a, auto b) {
        if (opcode == opcodes::LLSH_R)
          return a << b;
        else if (opcode == opcodes::ALSH_R) {
          using signed_a = std::make_signed_t<decltype(a)>;
          auto result = static_cast<signed_a>(a) << b;
          return static_cast<decltype(a)>(result);
        } else if (opcode == opcodes::LRSH_R)
          return a >> b;
        else if (opcode == opcodes::ARSH_R) {
          using signed_a = std::make_signed_t<decltype(a)>;
          auto result = static_cast<signed_a>(a) >> b;
          return static_cast<decltype(a)>(result);
        } else
          throw no_such_opcode("opcode does not exist in bit manip group");
      };
      auto [rd, rs, rr] = register_decode_dss<u32>(instruction);
      *rd = operation(*rs, *rr);
      set_needed_ctrl(rd);
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
      set_needed_ctrl(rd);
    } break;
    case opcodes::LLSH_I:
    case opcodes::ALSH_I:
    case opcodes::LRSH_I:
    case opcodes::ARSH_I: {
      auto operation = [&opcode](auto a, auto b) {
        if (opcode == opcodes::LLSH_I)
          return a << b;
        else if (opcode == opcodes::ALSH_I)
          return a << b;  // TODO: fix this
        else if (opcode == opcodes::LRSH_I)
          return a >> b;
        else if (opcode == opcodes::ARSH_I)
          return a >> b;  // TODO: fix
        else
          throw no_such_opcode("opcode does not exist in bit manip group");
      };
      auto [rd, rs] = register_decode_dsi<u32>(instruction);
      auto lit = literal_decode<8>(instruction);
      *rd = operation(*rs, lit);
      set_needed_ctrl(rd);
    } break;
    case opcodes::MOVE: {
      auto [rd, rs] = register_decode_dsi<u32>(instruction);
      *rd = *rs;
      set_needed_ctrl(rd);
    } break;
    case opcodes::NOT_R: {
      auto [rd, rs] = register_decode_dsi<u32>(instruction);
      *rd = ~*rs;
      set_needed_ctrl(rd);
    } break;
    case opcodes::LOAD_AT_ADDR: {
      metaout << "Loading from address... " << endl;
      auto [rd, rs] = register_decode_dsi<u32>(instruction);
      *rd = ram[*rs];
      metaout << " Loaded value at " << *rs << " is " << *rd << endl;
      set_needed_ctrl(rd);
    } break;
    case opcodes::STORE_AT_ADDR: {
      metaout << "Storing to address... " << endl;
      auto [rd, rs] = register_decode_dsi<u32>(instruction);
      ram[*rd] = *rs;
      metaout << " Stored value of " << *rs << " to " << *rd << endl;
      // set_needed_ctrl(rd);
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
    case opcodes::BNCH: {
      metaout << "Conditional branch... ";

      if (!ctrl_get(ctrl_bits::CTRL_TEST_TRUE)) {
        metaout << "... not taken " << endl;
        break;
      }
    } /* break; */
    case opcodes::JMP: {
      u32 addr = literal_decode<24>(instruction);
      std::cout << "Moving off by " << addr << " from " << pc << std::endl;
      pc = addr;

    } break;

    case opcodes::BNCH_WITH_OFFSET: {
      metaout << "Conditional branch... ";

      if (!ctrl_get(ctrl_bits::CTRL_TEST_TRUE)) {
        metaout << "... not taken " << endl;
        break;
      }
    } /* break; */
    // Do not separate the BNCH and JMP instructions
    // BNCH relies on JMP being the next case
    case opcodes::JMP_WITH_OFFSET: {
      metaout << " JUMP " << endl;
      u32 base = literal_decode<24>(instruction);
      auto offset = get_jump_offset(base);
      pc += offset;  // will be negative if first bit is set
      metaout << "pc is now at " << pc << " and is " << ram[pc] << endl;
    } break;
    case opcodes::HALT: {
      halted = true;
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
    case opcodes::TEST_CTRL_NEG: {
      if (ctrl_get(ctrl_bits::CTRL_NEG_BIT)) {
        ctrl_set(ctrl_bits::CTRL_TEST_TRUE);
      } else {
        ctrl_clear(ctrl_bits::CTRL_TEST_TRUE);
      }
    } break;
    case opcodes::POPCNT: {
      metaout << "CPU does have popcnt!" << endl;
      auto [result, src] = register_decode_dsi<u32>(instruction);
      // __builtin_popcount works on ARM64 Apple Silicon
      *result = __builtin_popcount(*src);
      // asm("movl %1, %%eax;"
      //     "popcntl %%eax, %%eax;"
      //     "movl %%eax, %0;"
      //     : "=r"(*result) /* output */
      //     : "r"(*src)     /* input */
      //     : "%eax"        /* clobbered register */
      // );
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
      metaout << "Setting " << *dest << " to " << *l << " op " << short_literal
              << endl;
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
    case opcodes::RND_SEED: {
      auto p = register_decode_first<u32>(instruction);
      srand(*p);
    } break;
    case opcodes::RND_NUM: {
      auto* p = register_decode_first<u32>(instruction);
      *p = rand();
      set_needed_ctrl(p);
    } break;
    case opcodes::GETC_R: {
      auto [destination, _] = register_decode_dsi<u32>(instruction);
      *destination = getchar();
      set_needed_ctrl(destination);
    } break;
    case opcodes::SQRT_R_I: {
      auto* s = register_decode_first<u32>(instruction);
      *s = static_cast<u32>(std::sqrt(*s));
      set_needed_ctrl(s);
    } break;
    case opcodes::EXT_INSTR: {
      ctrl_set(ctrl_bits::CTRL_EXT_FNC);
    } break;
    default: {
      std::stringstream msg;
      msg << "No such opcode " << static_cast<int>(opcode) << " in instruction "
          << instruction;
      throw no_such_opcode(msg.str());
    }
  }
}

void
cpu::execute_extended_instruction(u8 opcode, u32 instruction) {
  metaout << "extended_tick" << endl;
  ctrl_clear(ctrl_bits::CTRL_EXT_FNC);

  switch (opcode) {
    case extended_opcodes::FADD_DSI:
    case extended_opcodes::FSUB_DSI:
    case extended_opcodes::FMULT_DSI: {
      auto operation = [&opcode](auto a, auto b) {
        if (opcode == extended_opcodes::FADD_DSI)
          return a + b;
        else if (opcode == extended_opcodes::FSUB_DSI)
          return a - b;
        else if (opcode == extended_opcodes::FMULT_DSI)
          return a * b;
        else
          throw no_such_opcode("missing opcode in lambda for DSI");
      };
      auto [dest, l] = register_decode_dsi<f64>(instruction);
      auto short_literal = literal_decode<8, f64>(instruction);
      metaout << "Setting " << *dest << " to " << *l << " op " << short_literal
              << endl;
      *dest = operation(*l, short_literal);
      // set_needed_ctrl(dest);
    } break;
    case extended_opcodes::FMULT_DSS:
    case extended_opcodes::FSUB_DSS:
    case extended_opcodes::FADD_DSS: {
      auto operation = [&opcode](auto a, auto b) {
        if (opcode == extended_opcodes::FMULT_DSS)
          return a * b;
        else if (opcode == extended_opcodes::FSUB_DSS)
          return a - b;
        else if (opcode == extended_opcodes::FADD_DSS)
          return a + b;
        else
          throw no_such_opcode("missing opcode in lambda for DSS");
      };
      auto [dest, l, r] = register_decode_dss<f64>(instruction);
      metaout << "operating " << *l << " op " << *r << endl;
      // *dest = *l * *r;
      *dest = operation(*l, *r);
      // set_needed_ctrl(dest);
    } break;
    case extended_opcodes::FSQRT_R_I: {
      auto* s = register_decode_first<f64>(instruction);
      *s = static_cast<f64>(std::sqrt(*s));
      // set_needed_ctrl(s);
    } break;
    case extended_opcodes::LOAD_FIM_FA: {
      u32 bits = ((literal_decode<32, u32>(instruction) & ~0xff000000) << 8);
      f32 value = *(f32*)&bits;
      fa = value;
      metaout << "loading fa=" << fa << " from " << instruction;
    } break;
    case extended_opcodes::LOAD_FIM_FB: {
      u32 bits = ((literal_decode<32, u32>(instruction) & ~0xff000000) << 8);
      f32 value = *(f32*)&bits;
      fb = value;
      metaout << "loading fb=" << fb << " from " << instruction;
    } break;
    default: {
      throw no_such_opcode("The extended opcode does not exist");
    }
  }
}

}  // namespace emulator
