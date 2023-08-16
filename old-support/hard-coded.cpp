#include "emulator.hpp"

void
hard_coded(int argc, char const** argv) {
  /*****************************************************************
   * Run ./emulate 2>/dev/null to see ONLY emulated program output *
   *****************************************************************/

  if (argc >= 2 && std::string(argv[1]) == "-s")
    emulator::metaout = emulator::printer::nullprinter;

  emulator::cpuout = emulator::printer::nullprinter;
  emulator::metaout = emulator::printer::nullprinter;

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
                                         // this is the address of the string to
                                         // be printed
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::LOAD_AT_ADDR,  // b = *a
       0x02, 0x01, 0x00,
       emulator::cpu::opcodes::TEST_EQ,  // b == 0
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::BNCH_WITH_OFFSET,  // branch with offset
       // if the test succeeds a ctrl bit is set and the branch is taken
       // if the test fails then no branch is taken
       0x80, 0x00, 0x0c,
       emulator::cpu::opcodes::PUTC_R,  // putchar(b)
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::INC_A,  // ++a
       0x00, 0x00, 0x00,
       emulator::cpu::opcodes::JMP_WITH_OFFSET,  // loop back to start
       // jmp is an unconditional loop
       0x00, 0x00, 0x18,
       emulator::cpu::opcodes::RET,  // Return
       0x00, 0x00, 0x00

      }};

  std::string msg = "distance = ";

  std::vector<emulator::byte> dist = {
      {emulator::cpu::opcodes::REG_POP,  // pop arg1 into a
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::REG_POP,  // pop arg2 into b
       0x00, 0x00, 0x02,

       emulator::cpu::opcodes::REG_PUSH,  // save the ra
       // before calling a function you must save the return address
       // then push the arguments to the function then call the function
       0x00, 0x00, 0x05,
       emulator::cpu::opcodes::REG_PUSH,  // push a
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::REG_PUSH,  // push b
       0x00, 0x00, 0x02,
       // b is the top of the stack adn since square is a unary function
       // only b will be squared
       emulator::cpu::opcodes::CALL_FN_I,  // call square at 0x2000 for b
       0x00, 0x20, 0x00,
       emulator::cpu::opcodes::REG_POP,  // pop squared result into b
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::REG_POP,  // pop original arg1 result into a
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::REG_PUSH,  // push b*b to save it
       0x00, 0x00, 0x02,
       // the result of squaring b needs to be preserved before the next
       // function call so it is pushed to the stack but in reverse order so a
       // can be the top of the stack for the next call to square
       emulator::cpu::opcodes::REG_PUSH,  // push a to be squared
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::CALL_FN_I,  // call squrae at 0x2000 for a
       0x00, 0x20, 0x00,
       emulator::cpu::opcodes::REG_POP,  // pop fn result a*a into a
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::REG_POP,  // pop b*b into b
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::REG_POP,  // pop ra back into ra
       0x00, 0x00, 0x05,
       // function must restore ra after a function call before RET
       emulator::cpu::opcodes::ADD_DSS,  // int y = a + b
       0x03, 0x01, 0x02,
       emulator::cpu::opcodes::SQRT_R_I,  // (int)sqrt(y)
       0x00, 0x00, 0x03,
       emulator::cpu::opcodes::REG_PUSH,  // push y as result
       0x00, 0x00, 0x03,
       emulator::cpu::opcodes::RET,  // return;
       0x00, 0x00, 0x00}};

  std::vector<emulator::byte> data = {
      {emulator::cpu::opcodes::LD_IM_A,  // load a=3
       0x00, 0x00, 0x03,
       emulator::cpu::opcodes::REG_PUSH,  // push a as first argument to fn
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

       emulator::cpu::opcodes::PRINT_I_R,  // print the value of a as a int
       0x00, 0x00, 0x01,
       emulator::cpu::opcodes::LD_IM_B,  // load 10 into b
       0x00, 0x00, 0x0a,
       emulator::cpu::opcodes::PUTC_R,  // print the value of b as a char
       0x00, 0x00, 0x02,
       emulator::cpu::opcodes::HALT,  // halt
       0x00, 0x00, 0x00}};

  processor.set_memory(reinterpret_cast<emulator::byte*>(msg.data()),
                       msg.size(), 0x1000);
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
           emulator::cpu::opcodes::PRINT_I_R,  // print the value in a as an int
           0x00, 0x00, 0x01,
           emulator::cpu::opcodes::LD_IM_B,  // load 0x0a aka '\n' into b
           0x00, 0x00, 0x0a,
           emulator::cpu::opcodes::PUTC_R,  // print the value of b as a char
           0x00, 0x00, 0x02,
           emulator::cpu::opcodes::HALT,  // halt
           0x00, 0x00, 0x00}};

  processor.set_memory(data.data(), data.size(), 0xF000);
  processor.run();

  data = {{EXT_INSTR(LOAD_FIM_FA, 0x40, 0x4a, 0x11),
           EXT_INSTR(LOAD_FIM_FB, 0x41, 0x33, 0x11),
           EXT_INSTR(FADD_DSS, 0x03, 0x01, 0x02)}};
  processor.reset();
  processor.set_memory(data.data(), data.size(), 0xF000);
  processor.run();
  processor.dump_registers();

  data = {{
      emulator::cpu::opcodes::LD_IM_A,   0x11, 0x11, 0x00,
      emulator::cpu::opcodes::POPCNT,    0x01, 0x01, 0x00,
      emulator::cpu::opcodes::PRINT_I_R, 0x00, 0x00, 0x01,
      emulator::cpu::opcodes::LD_IM_A,   0x00, 0x00, 0x0a,
      emulator::cpu::opcodes::PUTC_R,    0x00, 0x00, 0x01,
      emulator::cpu::opcodes::HALT,      0x00, 0x00, 0x00,
  }};

  processor.reset();
  processor.set_memory(data.data(), data.size(), 0xF000);
  processor.run();
}
