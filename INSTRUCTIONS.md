# N-Emulator Instruction Set

This document will outline the instruction set of the N-Emulator and how to use them.

Currently, there is not specified format for executable files, and so machine instructions are to be
directly loaded into the cpu for execution. This will likely change in the future.

## Register Indices
In all instructions which use regsiters, registeres are encoded as indices in the following way

### For integral instruction

| register | index |
| --- | --- |
| z | 0 |
| a | 1 |
| b | 2 |
| x | 3 |
| sp | 4 |
| ra | 5 |

other registers cannot be accessed using instructions

## Integral Instructions

### MOVE - Move a value into a register

This instruction sets the contents of the destination register to the contents of the source register.
This is what is known as a DSI instruction. DSI instructions take either two registers (destination and source)
or two registers (destination and source) and an immediate. Because this instruction does not take an immediate,
it is also known as a DS instruction.

The format of the instruction is

`opcode`  `dest` `src` `ignored`

`0x01     0xDD    0xSS   0xXX`

The format of the `dest` and `src` operands are what are called **register indices**. They are
values between 0 and 5 inclusive. See above for a list of which register correspond to which values

As an example to set the contents of register b equal to that of register a you would write
`0x01 0x02 0x01 0x00` corresponding to `MOVE B A -`.

### AND_R - AND two registers together

This instruction performs bitwise AND on two regsiters and stores the result in a third. This
is known as a DSS instruction since it requires 3 registers and no immediates and performs an
operation that is then stored in a destination register. The format
of all DSS instructions is

`opcode`  `dest` `operand1` `operand2`

`0x01      0xDD     0xSS      0xRR`

Ctrl bits are set accordingly based on what ends up in the destination register.

### OR_R - OR two registers together

This instruction performs bitwise OR on two regsiters and stores the result in a third. This
instruction has the same format as `AND_R`

### XOR_R - XOR two registers together

This instruction performs bitwise XOR on two regsiters and stores the result in a third. This
instruction has the same format as `AND_R`

### AND_I - AND a register and an immediate together

This instruction performs bitwise AND on one register and an immediate value
and stores the result in a third. This is known as a DSI instruction since
it requires 2 registers and an immediates and performs an
operation that is then stored in a destination register. The format
is

`opcode`  `dest` `operand1` `8-bit-Immediate`

`0x01      0xDD     0xSS      0xII`

Ctrl bits are set accordingly based on what ends up in the destination register.

### OR_I - OR a register and an immediate together

This instruction performs bitwise OR on one register and an immediate value
and stores the result in a third. See `AND_I` for more on the operation

### XOR_I - XOR a register and an immediate together

This instruction performs bitwise XOR on one register and an immediate value
and stores the result in a third. See `AND_I` for more on the operation

### LD_IM_A - Load Immediate into A

This instruction loads a 24-bit immediate value into register A and sets ctrl bits accordingly

### LD_IM_B - Load Immediate into B

This instruction loads a 24-bit immediate value into register B and sets ctrl bits accordingly

### LD_IM_X - Load Immediate into X

This instruction loads a 24-bit immediate value into register X and sets ctrl bits accordingly
