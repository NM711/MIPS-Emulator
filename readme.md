

https://github.com/user-attachments/assets/4a983855-b881-4d41-8cf2-e0fc1989c2c6

# MIPS CORE EMULATOR


This project is meant to be a forkable framework for the emulation of a MIPS based chip implementation.
I implement most (if not all) MIPS I instructions. Below, I will write a table for each instruction I implemented.

For me, this project is meant to be a base to emulate an R2000 chip, which implements the MIPS I architecture, and other chips aswell.

**NOTE**: Make sure the body of the syscall table is replaced with the relevant syscalls for the system you are attempting to emulate.
I added some basic write and exit, pseudo syscalls for testing purposes.

### Implemented Instruction Table

| Name    | Opcode | Function | Instruction Type | Notes                                                                  |
|---------|--------|----------|------------------|------------------------------------------------------------------------|
| j       | 000010 | 000000   | J Type           |                                                                        |
| jal     | 000011 | 000000   | J Type           |                                                                        |
| lb      | 100000 | 000000   | I Type           |                                                                        |
| lbu     | 100100 | 000000   | I Type           |                                                                        |
| lh      | 100001 | 000000   | I Type           |                                                                        |
| lhu     | 100101 | 000000   | I Type           |                                                                        |
| lw      | 100011 | 000000   | I Type           |                                                                        |
| lui     | 001111 | 000000   | I Type           |                                                                        |
| sb      | 101000 | 000000   | I Type           |                                                                        |
| sh      | 101001 | 000000   | I Type           |                                                                        |
| sw      | 101011 | 000000   | I Type           |                                                                        |
| addi    | 001000 | 000000   | I Type           |                                                                        |
| addiu   | 001001 | 000000   | I Type           |                                                                        |
| blez    | 000110 | 000000   | I Type           |                                                                        |
| bgtz    | 000111 | 000000   | I Type           |                                                                        |
| beq     | 000100 | 000000   | I Type           |                                                                        |
| bne     | 000101 | 000000   | I Type           |                                                                        |
| xori    | 001110 | 000000   | I Type           |                                                                        |
| ori     | 001101 | 000000   | I Type           |                                                                        |
| sllv    | 000000 | 000100   | R Type           |                                                                        |
| srlv    | 000000 | 000110   | R Type           |                                                                        |
| srav    | 000000 | 000111   | R Type           |                                                                        |
| sll     | 000000 | 000000   | R Type           |                                                                        |
| srl     | 000000 | 000010   | R Type           |                                                                        |
| sltu    | 000000 | 101001   | R Type           |                                                                        |
| slt     | 000000 | 101010   | R Type           |                                                                        |
| addu    | 000000 | 100001   | R Type           |                                                                        |
| add     | 000000 | 100000   | R Type           |                                                                        |
| sub     | 000000 | 100010   | R Type           |                                                                        |
| subu    | 000000 | 100011   | R Type           |                                                                        |
| multu   | 000000 | 011001   | R Type           |                                                                        |
| mult    | 000000 | 011000   | R Type           |                                                                        |
| divu    | 000000 | 011011   | R Type           |                                                                        |
| div     | 000000 | 011010   | R Type           |                                                                        |
| jr      | 000000 | 001000   | R Type           |                                                                        |
| jalr    | 000000 | 001001   | R Type           |                                                                        |
| and     | 000000 | 100100   | R Type           |                                                                        |
| or      | 000000 | 100101   | R Type           |                                                                        |
| nor     | 000000 | 100111   | R Type           |                                                                        |
| xor     | 000000 | 100110   | R Type           |                                                                        |
| mtlo    | 000000 | 010011   | R Type           |                                                                        |
| mthi    | 000000 | 010001   | R Type           |                                                                        |
| mfhi    | 000000 | 010000   | R Type           |                                                                        |
| mflo    | 000000 | 010010   | R Type           |                                                                        |
| syscall | 000000 | 001100   | R Type           | Body should be replaced with switch/table for system you are targeting |
