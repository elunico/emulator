
#include "emulator.hpp"

int main() {
  /*****************************************************************
   * Run ./emulate 2>/dev/null to see ONLY emulated program output *
   *****************************************************************/
  emulator::cpu processor;

  std::vector<emulator::byte> square = {
      {emulator::cpu::opcodes::REG_POP,  // pop arg1 into a
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::MULT_DSS,  // a = a * a;
       0x01, 0x01, 0x01,
       emulator::cpu::opcodes::REG_PUSH,  // push ret val
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::RET,  // return to caller
       0x00, 0x00, 0x00}};

  std::vector<emulator::byte> puts = {
      {emulator::cpu::opcodes::REG_POP,  // pop arg1 into a
       0x00,
       0x00,
       0x01,
       emulator::cpu::opcodes::LOAD_AT_ADDR,  // b = *a
       0x02,
       0x01,
       0x00,
       emulator::cpu::opcodes::TEST_EQ,  // b == 0
       0x00,
       0x00,
       0x02,
       emulator::cpu::opcodes::BNCH_WITH_OFFSET,  // branch with offset
       0x80,
       0x00,
       0x0c,
       emulator::cpu::opcodes::PUTC_R,  // putc(b)
       0x00,
       0x00,
       0x02,
       emulator::cpu::opcodes::INC_A,  // ++a
       0x00,
       0x00,
       0x00,
       emulator::cpu::opcodes::JMP_WITH_OFFSET,  // loop back to start
       0x00,
       0x00,
       0x18,
       emulator::cpu::opcodes::RET,  // Return
       0x00,
       0x00,
       0x00

      }};

  std::vector<emulator::byte> msg = {{'d', 'i', 's', 't', ' ', '=', ' ', '\0'}};

  std::vector<emulator::byte> dist = {{
      emulator::cpu::opcodes::REG_POP,  // pop arg1 into a
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::REG_POP,  // pop arg2 into b
      0x00,
      0x00,
      0x02,

      emulator::cpu::opcodes::REG_PUSH,  // save the ra
      0x00,
      0x00,
      0x05,
      emulator::cpu::opcodes::REG_PUSH,  // push a
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::REG_PUSH,  // push b
      0x00,
      0x00,
      0x02,

      emulator::cpu::opcodes::CALL_FN_I,  // call squrae at 0x2000 for b
      0x00,
      0x20,
      0x00,
      emulator::cpu::opcodes::REG_POP,  // pop squared result into b
      0x00,
      0x00,
      0x02,
      emulator::cpu::opcodes::REG_POP,  // pop original arg1 result into a
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::REG_PUSH,  // push b*b to save it
      0x00,
      0x00,
      0x02,
      emulator::cpu::opcodes::REG_PUSH,  // push a to be squared
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::CALL_FN_I,  // call squrae at 0x2000 for a
      0x00,
      0x20,
      0x00,
      emulator::cpu::opcodes::REG_POP,  // pop fn result a*a into a
      0x00,
      0x00,
      0x01,
      emulator::cpu::opcodes::REG_POP,  // pop b*b into b
      0x00,
      0x00,
      0x02,
      emulator::cpu::opcodes::REG_POP,  // pop ra back into ra
      0x00,
      0x00,
      0x05,
      emulator::cpu::opcodes::ADD_DSS,  // mult y = a + b
      0x03,
      0x01,
      0x02,
      emulator::cpu::opcodes::EXT_INSTR,  // take extended instruction
      0x00,
      0x00,
      0x00,
      emulator::cpu::extended_opcodes::SQRT_R_I,  // (int)sqrt(y)
      0x00,
      0x00,
      0x03,
      emulator::cpu::opcodes::REG_PUSH,  // push y as result
      0x00,
      0x00,
      0x03,
      emulator::cpu::opcodes::RET  // return;
  }};

  std::vector<emulator::byte> data = {
      {emulator::cpu::opcodes::LD_IM_A,  // load a=3
       0x00, 0x00, 0x03,
       emulator::cpu::opcodes::REG_PUSH,  // push a as second argument to fn
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::LD_IM_A,  // load a=4
       0x00, 0x00, 0x04,
       emulator::cpu::opcodes::REG_PUSH,  // push a as second argument to fn
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::CALL_FN_I,  // call function at 0x6000
       0x00, 0x60, 0x00,
       // return value is on stack, but since we are calling another function
       // we leave it on the stack until the next function is done to protect
       // the value from get clobbered in a register

       emulator::cpu::opcodes::LD_IM_B,  // load string addr into b (0x1000)
       0x00, 0x10, 0x00,
       emulator::cpu::opcodes::REG_PUSH,  // push b as argument to puts
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::CALL_FN_I,  // call function at 0x5000
       0x00, 0x50, 0x00,

       // no return value to care about void puts(char* a);
       // but result of 0x6000 is still on the stack to protect it from next fn
       emulator::cpu::opcodes::REG_POP,  // get result from fn 0x6000 into a
       0x00, 0x00, 0x01,

       emulator::cpu::opcodes::PRINT_I_A,  // print the value of a as a int
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::LD_IM_B,  // load 10 into b
       0x00, 0x00, 0x0a,
       emulator::cpu::opcodes::PUTC_R,  // print the value of b as a char
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::HALT,  // halt
       0x00, 0x00, 0x00}};

  processor.set_memory(msg.data(), msg.size(), 0x1000);
  processor.set_memory(square.data(), square.size(), 0x2000);
  processor.set_memory(puts.data(), puts.size(), 0x5000);
  processor.set_memory(dist.data(), dist.size(), 0x6000);
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
           emulator::cpu::opcodes::PRINT_I_A,  // print the value in a as an
           0x00, 0x00, 0x00,
           emulator::cpu::opcodes::LD_IM_B,  // print the value in a as an int
           0x00, 0x00, 0x0a,
           emulator::cpu::opcodes::PUTC_R,  // print the value of b as a
           0x00, 0x00, 0x02,
           emulator::cpu::opcodes::HALT,  // halt
           0x00, 0x00, 0x00}};

  processor.set_memory(data.data(), data.size(), 0xF000);
  processor.run();
}
