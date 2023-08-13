
#include "emulator.hpp"

int main() {
  /*****************************************************************
   * Run ./emulate 2>/dev/null to see ONLY emulated program output *
   *****************************************************************/
  emulator::cpu processor;

  std::vector<emulator::byte> fn = {{
      emulator::cpu::opcodes::SUB_DSI,
      0x04,
      0x04,
      0x04,
      emulator::cpu::opcodes::REG_POP,  // pop arg1 into a
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::REG_POP,  // pop arg2 into b
      0x00,
      0x00,
      0x02,
      emulator::cpu::opcodes::ADD_DSI,
      0x04,
      0x04,
      0x04,
      emulator::cpu::opcodes::MULT_DSS,  // mult a * b = y
      0x03,
      0x01,
      0x02,
      emulator::cpu::opcodes::REG_PUSH,  // push y as result
      0x00,
      0x00,
      0x03,
      emulator::cpu::opcodes::RET  // return;
  }};

  std::vector<emulator::byte> data = {
      {emulator::cpu::opcodes::LD_IM_A,  // load a=17
       0x00,
       0x00,
       0x11,
       emulator::cpu::opcodes::REG_PUSH,  // push a as second argument to fn
       0x00,
       0x00,
       0x01,
       emulator::cpu::opcodes::LD_IM_A,  // load a=13
       0x00,
       0x00,
       0x0c,
       emulator::cpu::opcodes::REG_PUSH,  // push a as second argument to fn
       0x00,
       0x00,
       0x01,
       emulator::cpu::opcodes::CALL_FN_I,  // call function at 0x6000
       0x00,
       0x60,
       0x00,
       emulator::cpu::opcodes::REG_POP,  // get result from function into a
       0x00,
       0x00,
       0x01,
       emulator::cpu::opcodes::PRINT_I_A,  // print the value of b as a char
       0x00,
       0x00,
       0x01,
       emulator::cpu::opcodes::LD_IM_B,  // print the value in a as an int
       0x00,
       0x00,
       0x0a,
       emulator::cpu::opcodes::PUTC_R,  // print the value of b as a char
       0x00,
       0x00,
       0x02,
       emulator::cpu::opcodes::HALT,  // halt
       0x00,
       0x00,
       0x00}};

  processor.set_memory(fn.data(), fn.size(), 0x6000);
  processor.set_memory(data.data(), data.size(), 0xF000);

  processor.run();
  processor.reset();

  data = {{emulator::cpu::opcodes::LD_IM_A,  // load a=0x40
           0x00, 0x00, 0x40,
           emulator::cpu::opcodes::LD_IM_B,  // load b=0x50
           0x00, 0x00, 0x50,
           emulator::cpu::opcodes::INC_A,  // increment a
           0x00, 0x00, 0x00,
           emulator::cpu::opcodes::TEST_NEQ,  // test not equal a and b
           0x00, 0x02, 0x01,
           emulator::cpu::opcodes::BNCH_WITH_OFFSET,  // conditional branch 12
                                                      // bytes back
           0x00, 0x00, 0x0c,
           emulator::cpu::opcodes::PRINT_I_A,  // print the value in a as an int
           0x00, 0x00, 0x00,
           emulator::cpu::opcodes::LD_IM_B,  // print the value in a as an int
           0x00, 0x00, 0x0a,
           emulator::cpu::opcodes::PUTC_R,  // print the value of b as a char
           0x00, 0x00, 0x02,
           emulator::cpu::opcodes::HALT,  // halt
           0x00, 0x00, 0x00}};

  processor.set_memory(data.data(), data.size(), 0xF000);
  processor.run();
}
