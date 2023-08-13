# N-Emulator

## Warning
**This project and architecture is incomplete and highly unstable.**
**Anything in this project could fundamentally change at any time.**
**Not all instructions are possible, not all bugs are squashed, and not all features are used.**

This is a CPU emulator that was originally going to be based around the
MOS 6502, but it quickly became not that really at all.

## Architecture

This CPU has 32-bit registers, and 65536 bytes of byte-addressable RAM

  -  There are 3 GP registers called `a`, `b`, and `x`.
  -  There are 3 Floating Point registers `fa`, `fb`, and `fx`. Currenly no instructions use these
  -  There is a stack pointer (`sp`), return address (`ra`) and program counter (`pc`) registers.
  -  There is a zero register called `z` which is always 0. This is useful since there
     is a `TEST` instruction for equality and the zero register makes LT and GT
     easier to do.

## Calling convention

  -  The calling convention for function calls require that the caller
     preserve **all** registers including `ra` before calling a function
  -  Arguments are passed on the stack
  -  Return values are returned on the stack and typically limited to one though each
     function can define its own interface for how many return values it provides


There are many opcodes which run in 1 complete FDE cycle and then there are
extended instructions that take an extra cycle. These opcodes are accessed
by first running the `EXT_INSTR` opcode then placing the extended opcode as
the next instruction in the program

