#ifndef MIPS_I_VM
#define MIPS_I_VM

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
  Standalone MIPS Core that attempts to remain faithful to the MIPS I spec.
  Meant to be reusable in the event of emulating other MIPS based systems.
*/

class VirtualMachine {
private:
  enum ExtractKind { OP, RS, RT, RD, SHMT, FN, ADDR, IMMD };

  // 32, 32 bit general purpose registers.
  // Register 0, is the null register.
  // Register 31 (32nd element), is the jal register but can also be used for
  // other things.
  // Register 29 is the SP register, we do not touch it. Programmer sets that.
  std::array<unsigned, 32> registers;
  // MIPS memory is byte addressable max is managed in laad(), 2^32 - 1
  std::vector<unsigned char> memory;
  // Points to an individual byte in memory.
  unsigned ip;
  // hi holds the most significant 32 its of a 64 bit product from a
  // multiplication, or the remainder of division
  unsigned hi;
  // lo holds the least significant 32 bits of a product or the quotient
  unsigned lo;

  unsigned extract(unsigned value, ExtractKind kind) {
    switch (kind) {
    case OP:
      // op = 1111 1100 0000 0000 0000 0000 0000 0000 -> 0xFC000000
      return (value & 0xFC000000) >> 26;
    case RS:
      // rs = 0000 0011 1110 0000 0000 0000 0000 0000 -> 0x03E00000
      return (value & 0x03E00000) >> 21;
    case RT:
      // rt = 0000 0000 0001 1111 0000 0000 0000 0000 -> 0x001F0000
      return (value & 0x001F0000) >> 16;
    case RD:
      // rd = 0000 0000 0000 0000 1111 1000 0000 0000 -> 0x0000F800
      return (value & 0x0000F800) >> 11;
    case SHMT:
      // shamt = 0000 0000 0000 0000 0000 0111 1100 0000 -> 0x000007C0
      return (value & 0x000007C0) >> 6;
    case FN:
      // func = 0000 0000 0000 0000 0000 0000 0011 1111 -> 0x0000003F
      return (value & 0x0000003F);
    case ADDR:
      // addr = 0000 0011 1111 1111 1111 1111 1111 1111 -> 0x03FFFFFF
      return (value & 0x03FFFFFF) << 2;
    case IMMD:
      // immd = 0000 0000 0000 0000 1111 1111 1111 1111 -> 0x0000FFFF
      return (value & 0x0000FFFF);
    default:
      std::cerr << "Invalid ExtractKind issued" << std::endl;
      exit(1);
    };
  };

  void printRegisterContents() {
    for (int i = 0; i < 32; ++i) {
      std::cout << std::dec << "REGISTER #" << i << ": "
                << this->registers.at(i) << std::endl;
    }
  }

  // Fetches the next 32 instruction. Defaults to reading as Big Endiann
  unsigned fetch() {
    return (this->memory.at(this->ip) << 24) |
           (this->memory.at(this->ip + 1) << 16) |
           (this->memory.at(this->ip + 2) << 8) |
           (this->memory.at(this->ip + 3));
  };

  void cycle() {
    /*
      R TYPE = [op, rs, rt, rd, shamt, func]
      I TYPE = [op, rs, rt, immediate]
      J TYPE = [op, addr]
    */

    unsigned instruction = this->fetch();
    std::cout << "Instruction: " << std::showbase << std::hex << instruction
              << std::endl;
    std::cout << "OP: " << std::showbase << std::hex
              << this->extract(instruction, OP) << std::endl;
    // J Address = 0000 0011 1111 1111 1111 1111 1111 1111 = 0x03FFFFFF << 2
    switch (this->extract(instruction, OP)) {

    // ==== J TYPES ====
    // NOTE JUMP TYPES FORM A COMPLETE 32 BIT ADDRESS WHICH IS GRANTED TO THE
    // INSTRUCTION POINTER, BY ADDING THE TWO SHIFT PADDING AND THEN
    // CONCATENATING THE 28 BITS WITH THE UPPER 4 BITS OF THE INSTRUCTION
    // POINTER ITSELF. j -> 000010
    case 0x02:
      /*
        In the context of big-endian MIPS, the instruction's 26-bit target
        address is combined with the upper 4 bits of the current program counter
        (PC) and two zero bits to form the full 32-bit jump address
      */
      // 4 bits + 28 bits = 32 bit addr
      this->ip = (this->ip & 0xF0000000) | this->extract(instruction, ADDR);
      break;

    // jal -> 000011
    case 0x03:
      this->registers.at(31) = this->ip + 4;
      this->ip = (this->ip & 0xF0000000) | this->extract(instruction, ADDR);
      break;

      // ==== I TYPES ====

      // lb -> 100000

    case 0x20: {
      // Addr is the value of the source register, then we apply memory offset.
      // Also lb requires a signed constant AND register, NOT unsigned.
      // Since we are working with signed values, we are expecting a signed byte
      // to be returned from memory. Since the vector is unsigned we will cast
      // it to char.
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      char value = static_cast<char>(this->memory.at(address));

      this->registers.at(this->extract(instruction, RT)) = value;
      this->ip += 4;
      break;
    };

      // lbu -> 100100

    case 0x24: {
      unsigned address = registers.at(this->extract(instruction, RS)) +
                         this->extract(instruction, IMMD);
      unsigned char value = this->memory.at(address);
      this->registers.at(this->extract(instruction, RT)) = value;
      this->ip += 4;
      break;
    };

      // lh -> 100001

    case 0x21: {
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      short value = static_cast<short>((this->memory.at(address) << 8) |
                                       this->memory.at(address + 1));
      this->registers.at(this->extract(instruction, RT)) = value;
      this->ip += 4;
      break;
    };

      // lhu -> 100101

    case 0x25: {
      unsigned address = registers.at(this->extract(instruction, RS)) +
                         this->extract(instruction, IMMD);
      unsigned char value =
          (this->memory.at(address) << 8) | this->memory.at(address + 1);
      this->registers.at(this->extract(instruction, RT)) = value;
      this->ip += 4;
      break;
    };

      // lw -> 100011

    case 0x23: {
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      int value = static_cast<int>((this->memory.at(address) << 24) |
                                   (this->memory.at(address + 1) << 16) |
                                   (this->memory.at(address + 2) << 8) |
                                   this->memory.at(address + 3));
      this->registers.at(this->extract(instruction, RT)) = value;
      this->ip += 4;
      break;
    };

      // lui -> 001111

    case 0x0F: {

      // Shifts immediate value 16 bits and saves into destination register
      unsigned short value = this->extract(instruction, ExtractKind::IMMD)
                             << 16;
      this->registers.at(this->extract(instruction, RD)) = value;
      this->ip += 4;
      break;
    };

    // NOTE that while we expect the register address to be that of $sp, we
    // still explicitly extract the RT instead of hardcoding 29. NOTE This is
    // because its still considered "general purpose" and some people will write
    // programs that are not bounded to that convention.

    // sb -> 101000
    case 0x28: {
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      // gets implicitly casted to unsigned char, 8 bits zero signed.
      this->memory.at(address) =
          this->registers.at(this->extract(instruction, RT));
      this->ip += 4;
      break;
    };

    // sh -> 101001
    case 0x29: {
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      // save the first 8 bits in address, then save the other 8 bits in address
      // + 1
      unsigned rt = this->extract(instruction, RT);
      this->memory.at(address) = (this->registers.at(rt) & 0xFF00) >> 8;
      this->memory.at(address + 1) = (this->registers.at(rt) & 0x00FF);
      this->ip += 4;
      break;
    };

      // sw -> 101011
    case 0x2B: {
      int address =
          static_cast<int>(registers.at(this->extract(instruction, RS))) +
          static_cast<short>(this->extract(instruction, IMMD));
      unsigned rt = this->extract(instruction, RT);

      // same idea but working with 4 addresses
      this->memory.at(address) = (this->registers.at(rt) & 0xFF000000) >> 24;
      this->memory.at(address + 1) =
          (this->registers.at(rt) & 0x00FF0000) >> 16;
      this->memory.at(address + 2) = (this->registers.at(rt) & 0x0000FF00) >> 8;
      this->memory.at(address + 3) = this->registers.at(rt) & 0x000000FF;

      this->ip += 4;
      break;
    };

      // addi -> 001000

    case 0x08: {
      break;
    };

      // addiu -> 001001

    case 0x09: {
      /*
        The addiu instruction includes a 16-bit immediate operand. When the ALU
        executes the instruction, the immediate operand is sign-extended to 32
        bits. If two's complement overflow occurs during the addition, it is
        ignored. NOTE meaning we dont respond or act if there is an overflow or
        underflow unlike "addi" which raises a processor exception.
      */

      // My thought process is to truncate the 32 bit unsigned that is retrieved
      // from this->extract(), we do this by casting to short. Then we sign
      // extend the 16 bit short by 16 more bits, 32 total.
      int sum = static_cast<int>(
                    static_cast<short>(this->extract(instruction, IMMD))) +
                this->registers.at(this->extract(instruction, RS));
      this->registers.at(this->extract(instruction, RT)) = sum;

      std::cout << std::dec << sum << std::endl;

      this->ip += 4;
      break;
    };

      // NOTE ALL BRANCH INSTRUCTIONS PERFORM RELATIVE JUMPS MEANING, WE START
      // FROM (PC + 4) + IMMD << 2

      // blez -> 000110

    case 0x06: {

      int rs =
          static_cast<int>(this->registers.at(this->extract(instruction, RS)));

      if (rs <= 0) {
        this->ip +=
            4 + (static_cast<short>(this->extract(instruction, IMMD)) << 2);

      } else {
        this->ip += 4;
      }

      break;
    }

      // bgtz -> 000111

    case 0x07: {

      unsigned rs = this->registers.at(this->extract(instruction, RS));

      if (rs > 0) {
        this->ip +=
            4 + (static_cast<short>(this->extract(instruction, IMMD)) << 2);

      } else {
        this->ip += 4;
      }

      break;
    }

      // beq -> 000100

    case 0x04: {
      unsigned rs = this->registers.at(this->extract(instruction, RS));
      unsigned rt = this->registers.at(this->extract(instruction, RT));

      if (rs == rt) {
        this->ip +=
            4 + (static_cast<short>(this->extract(instruction, IMMD)) << 2);
      } else {
        this->ip += 4;
      }

      break;
    }

      // bne -> 000101

    case 0x05: {
      unsigned rs = this->registers.at(this->extract(instruction, RS));
      unsigned rt = this->registers.at(this->extract(instruction, RT));

      if (rs != rt) {
        this->ip +=
            4 + (static_cast<short>(this->extract(instruction, IMMD)) << 2);
      } else {
        this->ip += 4;
      }
      break;
    }

      // xori -> 001110

    case 0x0E: {
      unsigned rs = this->registers.at(this->extract(instruction, RS));
      short immd = static_cast<short>(extract(instruction, IMMD));
      this->registers.at(this->extract(instruction, RT)) = rs ^ immd;
      this->ip += 4;
      break;
    }

    // ==== R TYPES ====

    // All R TYPES start with OP 000000, the actual distinction is made in the 6
    // bit function field in the instruction.
    case 0x0: {
      switch (this->extract(instruction, FN)) {
        // srav -> 000111
        // NOTE before implementing digest the difference between srav and srlv
        // case 0x07: {
        //   unsigned rs = this->registers.at(this->extract(instruction, RS));
        //   unsigned rt = this->registers.at(this->extract(instruction, RT));
        //   this->registers.at(this->extract(instruction, RD)) = rt >> rs;
        //   this->ip += 4;
        //   break;
        // }

        // sll -> 000000

      case 0x0: {
        unsigned rt = this->registers.at(this->extract(instruction, RT));
        unsigned shmt = this->extract(instruction, SHMT);
        this->registers.at(this->extract(instruction, RD)) = rt << shmt;
        this->ip += 4;
        break;
      }

        // srl -> 000010

      case 0x02: {
        unsigned rt = this->registers.at(this->extract(instruction, RT));
        unsigned shmt = this->extract(instruction, SHMT);
        this->registers.at(this->extract(instruction, RD)) = rt >> shmt;
        this->ip += 4;
        break;
      }

        // sltu -> 101001

      case 0x29: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(this->extract(instruction, RT));

        unsigned &rd = this->registers.at(this->extract(instruction, RD));

        if (rs < rt) {
          rd = 1;
        } else {
          rd = 0;
        };

        this->ip += 4;
        break;
      };

        // slt -> 101010

      case 0x2A: {
        int rs = static_cast<int>(
            this->registers.at(this->extract(instruction, RS)));
        int rt = static_cast<int>(
            this->registers.at(this->extract(instruction, RT)));

        unsigned &rd = this->registers.at(this->extract(instruction, RD));

        if (rs < rt) {
          rd = 1;
        } else {
          rd = 0;
        };

        this->ip += 4;

        break;
      };
        // addu -> 100001

      case 0x21: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(this->extract(instruction, RT));
        unsigned sum = rs + rt;
        this->registers.at(this->extract(instruction, RD)) = sum;
        this->ip += 4;
        break;
      }
        // add -> 100000

      case 0x20: {
        int rs = static_cast<int>(
            this->registers.at(this->extract(instruction, RS)));
        int rt = static_cast<int>(
            this->registers.at(this->extract(instruction, RT)));

        int sum = rs + rt;

        this->registers.at(this->extract(instruction, RD)) = sum;
        this->ip += 4;
        break;
      };

        // multu -> 011001

      case 0x19: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(this->extract(instruction, RT));

        // since i have the luxury to do so we will use a long
        unsigned long result = rs * rt;
        // Saves higher 32
        this->hi = (result & 0xFFFFFFFF00000000) >> 32;
        // Saves lower 32
        this->lo = (result & 0x00000000FFFFFFFF);
        this->ip += 4;
        break;
      };

        // mult -> 011000

      case 0x18: {
        // cast rs and rt to signed type save result into a long
        int rs = static_cast<int>(
            this->registers.at(this->extract(instruction, RS)));
        int rt = static_cast<int>(
            this->registers.at(this->extract(instruction, RT)));

        // since i have the luxury to do so we will use a long
        long result = rs * rt;
        // Saves higher 32
        this->hi = (result & 0xFFFFFFFF00000000) >> 32;
        // Saves lower 32
        this->lo = (result & 0x00000000FFFFFFFF);
        this->ip += 4;
        break;
      }
        // divu -> 011011

      case 0x1B: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(this->extract(instruction, RT));

        this->hi = rs % rt;
        this->lo = rs / rt;
        this->ip += 4;
        break;
      };

        // div -> 011010

      case 0x1A: {
        int rs = static_cast<int>(
            this->registers.at(this->extract(instruction, RS)));
        int rt = static_cast<int>(
            this->registers.at(this->extract(instruction, RT)));
        // Should produce a 32 signed bit result, which should be implicitly
        // casted to 32 bit unsigned.
        this->hi = rs % rt;
        this->lo = rs / rt;
        this->ip += 4;
        break;
      }

        // jr -> 001000
      case 0x08:
        this->ip = this->registers.at(this->extract(instruction, RS));
        break;

      // jalr -> 001001
      case 0x09:
        // Save next address into rd reg, typically is $31 but we should still
        // decode seperately
        this->registers.at(this->extract(instruction, RD)) = this->ip + 4;
        this->ip = this->registers.at(this->extract(instruction, RS));
        break;
        // and -> 100100

      case 0x24: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(extract(instruction, RT));
        this->registers.at(this->extract(instruction, RD)) = rs & rt;
        this->ip += 4;
        break;
      }

        // or -> 100101

      case 0x25: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(extract(instruction, RT));
        this->registers.at(this->extract(instruction, RD)) = rs | rt;
        this->ip += 4;
        break;
      };

        // nor -> 100111
      case 0x27: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(extract(instruction, RT));
        this->registers.at(this->extract(instruction, RD)) = ~(rs | rt);
        this->ip += 4;
        break;
      }

        // xor -> 100110

      case 0x26: {
        unsigned rs = this->registers.at(this->extract(instruction, RS));
        unsigned rt = this->registers.at(extract(instruction, RT));
        this->registers.at(this->extract(instruction, RD)) = rs ^ rt;
        this->ip += 4;
        break;
      };

        // mtlo -> 010011

      case 0x13: {
        this->lo = this->registers.at(this->extract(instruction, RS));
        this->ip += 4;
        break;
      };

        // mthi -> 010001

      case 0x11: {
        this->hi = this->registers.at(this->extract(instruction, RS));
        this->ip += 4;
        break;
      };

        // mfhi -> 010000

      case 0x10: {
        this->registers.at(this->extract(instruction, RD)) = this->hi;
        this->ip += 4;
        break;
      }

        // mflo -> 010010

      case 0x12: {
        this->registers.at(this->extract(instruction, RD)) = this->lo;
        this->ip += 4;
        break;
      };

        // syscall -> 001100

      case 0x0C: {
        // NOTE Add kernel call subroutine here.
        // NOTE Add kernel call subroutine here.
        // NOTE Add kernel call subroutine here.

        this->ip += 4;
        break;
      }

      default:
        std::cout << "Unknown function: " << std::showbase << std::hex
                  << this->extract(instruction, FN) << std::endl;
        exit(1);
      };
      break;
    }

    default:
      std::cout << "Unknown opcode: " << std::showbase << std::hex
                << this->extract(instruction, OP) << std::endl;
      exit(1);
    };
  };

public:
  // ip can be set on construction because different systems. May pre allocate
  // part of an address space. Not all systems start at 0x0
  // note that we do NOT touch the sp, sp should be determined at runtime. It is
  // up to the programmoer to determine the call stack size.
  VirtualMachine(unsigned ip = 0) {
    this->registers.fill(0);
    this->ip = ip;
    this->lo = 0;
    this->hi = 0;
  };

  // Loads Binary into memory
  void load(std::string path) {
    std::ifstream file(path, std::ifstream::binary);

    if (!file) {
      std::cerr << "(Error) Failed to open binary file at \"" + path + "\"!\n"
                << std::endl;
      exit(1);
    };

    // Go to end so we can read the total size (its a stream)
    file.seekg(0, std::ios_base::end);
    // Read size
    std::streamsize size = file.tellg();
    // Fallback to start
    file.seekg(0, std::ios_base::beg);

    const unsigned MAX_ADDRESSABLE_MEMORY = (1024u * 1024u * 1024u * 4u) - 1;

    if (size > MAX_ADDRESSABLE_MEMORY) {
      std::cerr << "(Error) Attempted to load a binary file into memory, that "
                   "goes beyond the max addressable space 4GB!\n"
                << std::endl;
      exit(1);
    };

    // Gotta resize it or else this->memory.data() will point to an invalid
    // address causing a segfault in file.read() Note that this only allocates
    // enough for static memory. We have not taken the stack into account.
    this->memory.resize(size);

    file.read(reinterpret_cast<char *>(this->memory.data()), size);

    if (!file) {
      std::cerr << "(Error) Failed to load binary into memory at \"" + path +
                       "\"!\n"
                << std::endl;
      exit(1);
    };
    std::cout << "(Success) Loaded binary into memory, beginning execution..."
              << std::endl;
  };

  void execute() {

    for (int i = 0; i < 100; ++i) {
      this->cycle();
    }

    // this->printRegisterContents();
    // while (true) {
    //   this->cycle();
    // };
  };
};

#endif
