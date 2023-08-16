#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include "bytedefs.hpp"
#include "cpu.hpp"
#include "memory.hpp"

#define EXT_INSTR(op, b, c, d)                         \
  emulator::cpu::opcodes::EXT_INSTR, 0x00, 0x00, 0x00, \
      emulator::cpu::extended_opcodes::op, b, c, d

namespace emulator {}  // namespace emulator

#endif
